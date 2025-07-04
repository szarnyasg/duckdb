#include "duckdb/catalog/duck_catalog.hpp"
#include "duckdb/catalog/dependency_manager.hpp"
#include "duckdb/catalog/catalog_entry/duck_schema_entry.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"
#include "duckdb/parser/parsed_data/create_schema_info.hpp"
#include "duckdb/catalog/default/default_schemas.hpp"
#include "duckdb/function/built_in_functions.hpp"
#include "duckdb/main/attached_database.hpp"
#include "duckdb/transaction/duck_transaction_manager.hpp"
#include "duckdb/function/function_list.hpp"

namespace duckdb {

DuckCatalog::DuckCatalog(AttachedDatabase &db)
    : Catalog(db), dependency_manager(make_uniq<DependencyManager>(*this)),
      schemas(make_uniq<CatalogSet>(*this, IsSystemCatalog() ? make_uniq<DefaultSchemaGenerator>(*this) : nullptr)) {
}

DuckCatalog::~DuckCatalog() {
}

void DuckCatalog::Initialize(bool load_builtin) {
	// first initialize the base system catalogs
	// these are never written to the WAL
	// we start these at 1 because deleted entries default to 0
	auto data = CatalogTransaction::GetSystemTransaction(GetDatabase());

	// create the default schema
	CreateSchemaInfo info;
	info.schema = DEFAULT_SCHEMA;
	info.internal = true;
	info.on_conflict = OnCreateConflict::IGNORE_ON_CONFLICT;
	CreateSchema(data, info);

	if (load_builtin) {
		BuiltinFunctions builtin(data, *this);
		builtin.Initialize();

		// initialize default functions
		FunctionList::RegisterFunctions(*this, data);
	}

	Verify();
}

bool DuckCatalog::IsDuckCatalog() {
	return true;
}

optional_ptr<DependencyManager> DuckCatalog::GetDependencyManager() {
	return dependency_manager.get();
}

//===--------------------------------------------------------------------===//
// Schema
//===--------------------------------------------------------------------===//
optional_ptr<CatalogEntry> DuckCatalog::CreateSchemaInternal(CatalogTransaction transaction, CreateSchemaInfo &info) {
	LogicalDependencyList dependencies;

	if (!info.internal && DefaultSchemaGenerator::IsDefaultSchema(info.schema)) {
		return nullptr;
	}
	auto entry = make_uniq<DuckSchemaEntry>(*this, info);
	auto result = entry.get();
	if (!schemas->CreateEntry(transaction, info.schema, std::move(entry), dependencies)) {
		return nullptr;
	}
	return result;
}

optional_ptr<CatalogEntry> DuckCatalog::CreateSchema(CatalogTransaction transaction, CreateSchemaInfo &info) {
	D_ASSERT(!info.schema.empty());
	auto result = CreateSchemaInternal(transaction, info);
	if (!result) {
		switch (info.on_conflict) {
		case OnCreateConflict::ERROR_ON_CONFLICT:
			throw CatalogException::EntryAlreadyExists(CatalogType::SCHEMA_ENTRY, info.schema);
		case OnCreateConflict::REPLACE_ON_CONFLICT: {
			DropInfo drop_info;
			drop_info.type = CatalogType::SCHEMA_ENTRY;
			drop_info.catalog = info.catalog;
			drop_info.name = info.schema;
			DropSchema(transaction, drop_info);
			result = CreateSchemaInternal(transaction, info);
			if (!result) {
				throw InternalException("Failed to create schema entry in CREATE_OR_REPLACE");
			}
			break;
		}
		case OnCreateConflict::IGNORE_ON_CONFLICT:
			break;
		default:
			throw InternalException("Unsupported OnCreateConflict for CreateSchema");
		}
		return nullptr;
	}
	return result;
}

void DuckCatalog::DropSchema(CatalogTransaction transaction, DropInfo &info) {
	D_ASSERT(!info.name.empty());
	if (!schemas->DropEntry(transaction, info.name, info.cascade)) {
		if (info.if_not_found == OnEntryNotFound::THROW_EXCEPTION) {
			throw CatalogException::MissingEntry(CatalogType::SCHEMA_ENTRY, info.name, string());
		}
	}
}

void DuckCatalog::DropSchema(ClientContext &context, DropInfo &info) {
	DropSchema(GetCatalogTransaction(context), info);
}

void DuckCatalog::ScanSchemas(ClientContext &context, std::function<void(SchemaCatalogEntry &)> callback) {
	schemas->Scan(GetCatalogTransaction(context),
	              [&](CatalogEntry &entry) { callback(entry.Cast<SchemaCatalogEntry>()); });
}

void DuckCatalog::ScanSchemas(std::function<void(SchemaCatalogEntry &)> callback) {
	schemas->Scan([&](CatalogEntry &entry) { callback(entry.Cast<SchemaCatalogEntry>()); });
}

CatalogSet &DuckCatalog::GetSchemaCatalogSet() {
	return *schemas;
}

optional_ptr<SchemaCatalogEntry> DuckCatalog::LookupSchema(CatalogTransaction transaction,
                                                           const EntryLookupInfo &schema_lookup,
                                                           OnEntryNotFound if_not_found) {
	auto &schema_name = schema_lookup.GetEntryName();
	D_ASSERT(!schema_name.empty());
	auto entry = schemas->GetEntry(transaction, schema_name);
	if (!entry) {
		if (if_not_found == OnEntryNotFound::THROW_EXCEPTION) {
			throw CatalogException(schema_lookup.GetErrorContext(), "Schema with name %s does not exist!", schema_name);
		}
		return nullptr;
	}
	return &entry->Cast<SchemaCatalogEntry>();
}

DatabaseSize DuckCatalog::GetDatabaseSize(ClientContext &context) {
	auto &transaction = DuckTransactionManager::Get(db);
	auto lock = transaction.SharedCheckpointLock();
	return db.GetStorageManager().GetDatabaseSize();
}

vector<MetadataBlockInfo> DuckCatalog::GetMetadataInfo(ClientContext &context) {
	auto &transaction = DuckTransactionManager::Get(db);
	auto lock = transaction.SharedCheckpointLock();
	return db.GetStorageManager().GetMetadataInfo();
}

bool DuckCatalog::InMemory() {
	return db.GetStorageManager().InMemory();
}

string DuckCatalog::GetDBPath() {
	return db.GetStorageManager().GetDBPath();
}

void DuckCatalog::Verify() {
#ifdef DEBUG
	Catalog::Verify();
	schemas->Verify(*this);
#endif
}

optional_idx DuckCatalog::GetCatalogVersion(ClientContext &context) {
	auto &transaction_manager = DuckTransactionManager::Get(db);
	auto transaction = GetCatalogTransaction(context);
	D_ASSERT(transaction.transaction);
	return transaction_manager.GetCatalogVersion(*transaction.transaction);
}

//===--------------------------------------------------------------------===//
// Encryption
//===--------------------------------------------------------------------===//
void DuckCatalog::SetEncryptionKeyId(const string &key_id) {
	encryption_key_id = key_id;
}

string &DuckCatalog::GetEncryptionKeyId() {
	return encryption_key_id;
}

void DuckCatalog::SetIsEncrypted() {
	is_encrypted = true;
}

bool DuckCatalog::GetIsEncrypted() {
	return is_encrypted;
}

} // namespace duckdb
