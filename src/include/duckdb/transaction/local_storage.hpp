//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/transaction/local_storage.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/types/chunk_collection.hpp"
#include "duckdb/storage/table/scan_state.hpp"
#include "duckdb/storage/index.hpp"
#include "duckdb/storage/table/version_manager.hpp"

namespace duckdb {
class DataTable;
class WriteAheadLog;
struct TableAppendState;

class LocalTableStorage {
public:
	LocalTableStorage(Transaction &transaction, StorageManager &storage, DataTable &table);
	~LocalTableStorage();

	Transaction &transaction;
	StorageManager &storage;
	DataTable &table;
	//! The columns collection holding the data
	vector<unique_ptr<ColumnData>> columns;
	//! The set of unique indexes
	vector<unique_ptr<Index>> indexes;
	 //! The set of deleted entries
	unordered_map<idx_t, unique_ptr<bool[]>> deleted_entries;
	//! The amount of rows in the table storage
	idx_t max_row;
public:
	void InitializeScan(LocalScanState &state);
	void Append(DataChunk &chunk);

	void Delete(Vector &row_ids, idx_t count);

	void Clear();
};

//! The LocalStorage class holds appends that have not been committed yet
class LocalStorage {
public:
	struct CommitState {
		unordered_map<DataTable *, unique_ptr<TableAppendState>> append_states;
	};

public:
	LocalStorage(Transaction &transaction, StorageManager &storage) : transaction(transaction), storage(storage) {}

	//! Initialize a scan of the local storage
	void InitializeScan(DataTable *table, LocalScanState &state);
	//! Scan
	void Scan(LocalScanState &state, const vector<column_t> &column_ids, DataChunk &result,
	          unordered_map<idx_t, vector<TableFilter>> *table_filters = nullptr);

	//! Append a chunk to the local storage
	void Append(DataTable *table, DataChunk &chunk);
	//! Delete a set of rows from the local storage
	void Delete(DataTable *table, Vector &row_ids, idx_t count);
	//! Update a set of rows in the local storage
	void Update(DataTable *table, Vector &row_ids, vector<column_t> &column_ids, DataChunk &data);

	//! Commits the local storage, writing it to the WAL and completing the commit
	void Commit(LocalStorage::CommitState &commit_state, Transaction &transaction, WriteAheadLog *log,
	            transaction_t commit_id);
	//! Revert the commit made so far by the LocalStorage
	void RevertCommit(LocalStorage::CommitState &commit_state);

	bool ChangesMade() noexcept {
		return table_storage.size() > 0;
	}

	void AddColumn(DataTable *old_dt, DataTable *new_dt, ColumnDefinition &new_column, Expression *default_value);
	void ChangeType(DataTable *old_dt, DataTable *new_dt, idx_t changed_idx, LogicalType target_type,
	                vector<column_t> bound_columns, Expression &cast_expr);

private:
	LocalTableStorage *GetStorage(DataTable *table);

	template <class T> bool ScanTableStorage(DataTable *table, LocalTableStorage *storage, T &&fun);

private:
	Transaction &transaction;
	StorageManager &storage;
	unordered_map<DataTable *, unique_ptr<LocalTableStorage>> table_storage;
};

} // namespace duckdb
