#include "duckdb/transaction/local_storage.hpp"
#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/storage/table/append_state.hpp"
#include "duckdb/storage/write_ahead_log.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/storage/uncompressed_segment.hpp"
#include "duckdb/storage/storage_manager.hpp"

namespace duckdb {
using namespace std;

LocalTableStorage::LocalTableStorage(Transaction &transaction, StorageManager &storage, DataTable &table) :
        transaction(transaction), storage(storage), table(table), max_row(0) {
	for (auto &index : table.info->indexes) {
		assert(index->type == IndexType::ART);
		auto &art = (ART &)*index;
		if (art.is_unique) {
			// unique index: create a local ART index that maintains the same unique constraint
			vector<unique_ptr<Expression>> unbound_expressions;
			for (auto &expr : art.unbound_expressions) {
				unbound_expressions.push_back(expr->Copy());
			}
			indexes.push_back(make_unique<ART>(art.column_ids, move(unbound_expressions), true));
		}
	}
}

LocalTableStorage::~LocalTableStorage() {
}

void LocalTableStorage::InitializeScan(LocalScanState &state) {
	state.storage = this;

	state.current_row = 0;
	state.max_row = max_row;
	if (columns.size() > 0) {
		state.column_scans = unique_ptr<ColumnScanState[]>(new ColumnScanState[columns.size()]);
		for(idx_t i = 0; i < columns.size(); i++) {
			columns[i]->InitializeScan(state.column_scans[i]);
		}
	}
}

void LocalTableStorage::Clear() {
	columns.clear();
	indexes.clear();
	max_row = 0;
}

void LocalTableStorage::Append(DataChunk &chunk) {
	if (columns.size() == 0) {
		for(idx_t i = 0; i < chunk.column_count(); i++) {
			columns.push_back(make_unique<ColumnData>(*storage.buffer_manager, *table.info));
			columns[i]->persistent_rows = MAX_ROW_ID;
			columns[i]->type = chunk.data[i].type;
			columns[i]->column_idx = i;
		}
	}
	for(idx_t i = 0; i < chunk.column_count(); i++) {
		ColumnAppendState append_state;
		columns[i]->InitializeAppend(append_state);
		columns[i]->Append(append_state, chunk.data[i], chunk.size());
	}
	max_row += chunk.size();
}

void LocalStorage::InitializeScan(DataTable *table, LocalScanState &state) {
	auto entry = table_storage.find(table);
	if (entry == table_storage.end()) {
		// no local storage for table: set scan to nullptr
		state.storage = nullptr;
		return;
	}
	state.storage = entry->second.get();
	state.storage->InitializeScan(state);
}

void LocalStorage::Scan(LocalScanState &state, const vector<column_t> &column_ids, DataChunk &result,
                        unordered_map<idx_t, vector<TableFilter>> *table_filters) {
	if (!state.storage || state.current_row > state.max_row) {
		// nothing left to scan
		result.Reset();
		return;
	}

	idx_t vector_idx = state.current_row % STANDARD_VECTOR_SIZE;
	idx_t chunk_count = MinValue<idx_t>(STANDARD_VECTOR_SIZE, state.max_row - state.current_row);
	idx_t count = chunk_count;

	// first create a selection vector from the deleted entries (if any)
	SelectionVector valid_sel(STANDARD_VECTOR_SIZE);
	auto entry = state.storage->deleted_entries.find(vector_idx);
	if (entry != state.storage->deleted_entries.end()) {
		// deleted entries! create a selection vector
		auto deleted = entry->second.get();
		idx_t new_count = 0;
		for (idx_t i = 0; i < count; i++) {
			if (!deleted[i]) {
				valid_sel.set_index(new_count++, i);
			}
		}
		if (new_count == 0 && count > 0) {
			// all entries in this chunk were deleted: continue to next chunk
			state.current_row += STANDARD_VECTOR_SIZE;
			Scan(state, column_ids, result, table_filters);
			return;
		}
		count = new_count;
	}

	SelectionVector sel;
	if (count != chunk_count) {
		sel.Initialize(valid_sel);
	} else {
		sel.Initialize(FlatVector::IncrementalSelectionVector);
	}
	// now scan the vectors of the chunk
	for (idx_t i = 0; i < column_ids.size(); i++) {
		auto id = column_ids[i];
		if (id == COLUMN_IDENTIFIER_ROW_ID) {
			// row identifier: return a sequence of rowids starting from MAX_ROW_ID plus the row offset in the chunk
			result.data[i].Sequence(MAX_ROW_ID + state.current_row, 1);
		} else {
			state.storage->columns[i]->Scan(transaction, state.column_scans[i], result.data[i]);
		}
		idx_t approved_tuple_count = count;
		if (table_filters) {
			auto column_filters = table_filters->find(i);
			if (column_filters != table_filters->end()) {
				//! We have filters to apply here
				for (auto &column_filter : column_filters->second) {
					nullmask_t nullmask = FlatVector::Nullmask(result.data[i]);
					UncompressedSegment::filterSelection(sel, result.data[i], column_filter, approved_tuple_count,
					                                     nullmask);
				}
				count = approved_tuple_count;
			}
		}
	}
	if (count == 0) {
		// all entries in this chunk were filtered:: Continue on next chunk
		state.current_row += STANDARD_VECTOR_SIZE;
		Scan(state, column_ids, result, table_filters);
		return;
	}
	if (count != chunk_count) {
		result.Slice(sel, count);
	} else {
		result.SetCardinality(count);
	}
	state.current_row += STANDARD_VECTOR_SIZE;
}

void LocalStorage::Append(DataTable *table, DataChunk &chunk) {
	auto entry = table_storage.find(table);
	LocalTableStorage *tstorage;
	if (entry == table_storage.end()) {
		auto new_storage = make_unique<LocalTableStorage>(transaction, storage, *table);
		tstorage = new_storage.get();
		table_storage.insert(make_pair(table, move(new_storage)));
	} else {
		tstorage = entry->second.get();
	}
	// append to unique indices (if any)
	if (tstorage->indexes.size() > 0) {
		idx_t base_id = MAX_ROW_ID + tstorage->max_row;

		// first generate the vector of row identifiers
		Vector row_ids(LOGICAL_ROW_TYPE);
		VectorOperations::GenerateSequence(row_ids, chunk.size(), base_id, 1);

		// now append the entries to the indices
		for (auto &index : tstorage->indexes) {
			if (!index->Append(chunk, row_ids)) {
				throw ConstraintException("PRIMARY KEY or UNIQUE constraint violated: duplicated key");
			}
		}
	}

	//! Append to the chunk
	tstorage->Append(chunk);
}

LocalTableStorage *LocalStorage::GetStorage(DataTable *table) {
	auto entry = table_storage.find(table);
	assert(entry != table_storage.end());
	return entry->second.get();
}

static idx_t GetChunk(Vector &row_ids) {
	auto ids = FlatVector::GetData<row_t>(row_ids);
	auto first_id = ids[0] - MAX_ROW_ID;

	return first_id / STANDARD_VECTOR_SIZE;
}

void LocalStorage::Delete(DataTable *table, Vector &row_ids, idx_t count) {
	auto storage = GetStorage(table);
	// figure out the chunk from which these row ids came
	idx_t chunk_idx = GetChunk(row_ids);
	assert(chunk_idx < storage->max_row % STANDARD_VECTOR_SIZE);

	// get a pointer to the deleted entries for this chunk
	bool *deleted;
	auto entry = storage->deleted_entries.find(chunk_idx);
	if (entry == storage->deleted_entries.end()) {
		// nothing deleted yet, add the deleted entries
		auto del_entries = unique_ptr<bool[]>(new bool[STANDARD_VECTOR_SIZE]);
		memset(del_entries.get(), 0, sizeof(bool) * STANDARD_VECTOR_SIZE);
		deleted = del_entries.get();
		storage->deleted_entries.insert(make_pair(chunk_idx, move(del_entries)));
	} else {
		deleted = entry->second.get();
	}

	// now actually mark the entries as deleted in the deleted vector
	idx_t base_index = MAX_ROW_ID + chunk_idx * STANDARD_VECTOR_SIZE;

	auto ids = FlatVector::GetData<row_t>(row_ids);
	for (idx_t i = 0; i < count; i++) {
		auto id = ids[i] - base_index;
		deleted[id] = true;
	}
}

void LocalStorage::Update(DataTable *table, Vector &row_ids, vector<column_t> &column_ids, DataChunk &data) {
	auto storage = GetStorage(table);
	for (idx_t i = 0; i < column_ids.size(); i++) {
		auto col_idx = column_ids[i];
		storage->columns[col_idx]->UpdateInPlace(data.data[i], row_ids, data.size());
	}
}

template <class T> bool LocalStorage::ScanTableStorage(DataTable *table, LocalTableStorage *storage, T &&fun) {
	vector<column_t> column_ids;
	for (idx_t i = 0; i < table->types.size(); i++) {
		column_ids.push_back(i);
	}

	DataChunk chunk;
	chunk.Initialize(table->types);

	// initialize the scan
	LocalScanState state;
	storage->InitializeScan(state);

	while (true) {
		Scan(state, column_ids, chunk);
		if (chunk.size() == 0) {
			return true;
		}
		if (!fun(chunk)) {
			return false;
		}
	}
}

void LocalStorage::Commit(LocalStorage::CommitState &commit_state, Transaction &transaction, WriteAheadLog *log,
                          transaction_t commit_id) {
	// commit local storage, iterate over all entries in the table storage map
	for (auto &entry : table_storage) {
		auto table = entry.first;
		auto storage = entry.second.get();

		// initialize the append state
		auto append_state_ptr = make_unique<TableAppendState>();
		auto &append_state = *append_state_ptr;
		// add it to the set of append states
		commit_state.append_states[table] = move(append_state_ptr);
		table->InitializeAppend(append_state);

		if (log && !table->info->IsTemporary()) {
			log->WriteSetTable(table->info->schema, table->info->table);
		}

		// scan all chunks in this storage
		ScanTableStorage(table, storage, [&](DataChunk &chunk) -> bool {
			// append this chunk to the indexes of the table
			if (!table->AppendToIndexes(append_state, chunk, append_state.current_row)) {
				throw ConstraintException("PRIMARY KEY or UNIQUE constraint violated: duplicated key");
			}

			// append to base table
			table->Append(transaction, commit_id, chunk, append_state);
			// if there is a WAL, write the chunk to there as well
			if (log && !table->info->IsTemporary()) {
				log->WriteInsert(chunk);
			}
			return true;
		});
	}
	// finished commit: clear local storage
	for (auto &entry : table_storage) {
		entry.second->Clear();
	}
	table_storage.clear();
}

void LocalStorage::RevertCommit(LocalStorage::CommitState &commit_state) {
	for (auto &entry : commit_state.append_states) {
		auto table = entry.first;
		auto storage = table_storage[table].get();
		auto &append_state = *entry.second;
		if (table->info->indexes.size() > 0 && !table->info->IsTemporary()) {
			row_t current_row = append_state.row_start;
			// remove the data from the indexes, if there are any indexes
			ScanTableStorage(table, storage, [&](DataChunk &chunk) -> bool {
				// append this chunk to the indexes of the table
				table->RemoveFromIndexes(append_state, chunk, current_row);

				current_row += chunk.size();
				if (current_row >= append_state.current_row) {
					// finished deleting all rows from the index: abort now
					return false;
				}
				return true;
			});
		}

		table->RevertAppend(*entry.second);
	}
}

void LocalStorage::AddColumn(DataTable *old_dt, DataTable *new_dt, ColumnDefinition &new_column,
                             Expression *default_value) {
	// check if there are any pending appends for the old version of the table
	auto entry = table_storage.find(old_dt);
	if (entry == table_storage.end()) {
		return;
	}
	// take over the storage from the old entry
	auto new_storage = move(entry->second);

	// now add the new column filled with the default value to all chunks
	auto new_column_type = new_column.type;

	auto new_column_data = make_unique<ColumnData>(*storage.buffer_manager, *new_dt->info);
	new_column_data->persistent_rows = MAX_ROW_ID;
	new_dt->AddColumn(*new_column_data, new_storage->columns.size(), new_column_type, default_value, new_storage->max_row);
	new_storage->columns.push_back(move(new_column_data));

	table_storage.erase(entry);
	table_storage[new_dt] = move(new_storage);
}

void LocalStorage::ChangeType(DataTable *old_dt, DataTable *new_dt, idx_t changed_idx, LogicalType target_type,
                              vector<column_t> bound_columns, Expression &cast_expr) {
	// check if there are any pending appends for the old version of the table
	auto entry = table_storage.find(old_dt);
	if (entry == table_storage.end()) {
		return;
	}
	throw NotImplementedException("FIXME: ALTER TYPE with transaction local data not currently supported");
}

} // namespace duckdb
