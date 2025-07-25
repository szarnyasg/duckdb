//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/operator/persistent/physical_batch_insert.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/operator/persistent/physical_insert.hpp"

namespace duckdb {

class PhysicalBatchInsert : public PhysicalOperator {
public:
	static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::BATCH_INSERT;

public:
	//! INSERT INTO
	PhysicalBatchInsert(PhysicalPlan &physical_plan, vector<LogicalType> types, TableCatalogEntry &table,
	                    vector<unique_ptr<BoundConstraint>> bound_constraints, idx_t estimated_cardinality);
	//! CREATE TABLE AS
	PhysicalBatchInsert(PhysicalPlan &physical_plan, LogicalOperator &op, SchemaCatalogEntry &schema,
	                    unique_ptr<BoundCreateTableInfo> info, idx_t estimated_cardinality);

	//! The table to insert into
	optional_ptr<TableCatalogEntry> insert_table;
	//! The insert types
	vector<LogicalType> insert_types;
	//! The bound constraints for the table
	vector<unique_ptr<BoundConstraint>> bound_constraints;
	//! Table schema, in case of CREATE TABLE AS
	optional_ptr<SchemaCatalogEntry> schema;
	//! Create table info, in case of CREATE TABLE AS
	unique_ptr<BoundCreateTableInfo> info;

public:
	// Source interface
	SourceResultType GetData(ExecutionContext &context, DataChunk &chunk, OperatorSourceInput &input) const override;

	bool IsSource() const override {
		return true;
	}

public:
	// Sink interface
	unique_ptr<GlobalSinkState> GetGlobalSinkState(ClientContext &context) const override;
	unique_ptr<LocalSinkState> GetLocalSinkState(ExecutionContext &context) const override;
	SinkNextBatchType NextBatch(ExecutionContext &context, OperatorSinkNextBatchInput &input) const override;
	SinkResultType Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const override;
	SinkCombineResultType Combine(ExecutionContext &context, OperatorSinkCombineInput &input) const override;
	SinkFinalizeType Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
	                          OperatorSinkFinalizeInput &input) const override;

	OperatorPartitionInfo RequiredPartitionInfo() const override {
		return OperatorPartitionInfo::BatchIndex();
	}

	bool IsSink() const override {
		return true;
	}

	bool ParallelSink() const override {
		return true;
	}

private:
	bool ExecuteTask(ClientContext &context, GlobalSinkState &gstate_p, LocalSinkState &lstate_p) const;
	void ExecuteTasks(ClientContext &context, GlobalSinkState &gstate_p, LocalSinkState &lstate_p) const;
};

} // namespace duckdb
