#include "duckdb/function/scalar/generic_functions.hpp"

#include <iostream>

namespace duckdb {

namespace {

struct ErrorOperator {
	template <class TA, class TR>
	static inline TR Operation(const TA &input) {
		throw InvalidInputException(input.GetString());
	}
};

} // namespace

ScalarFunction ErrorFun::GetFunction() {
	auto fun = ScalarFunction("error", {LogicalType::VARCHAR}, LogicalType::SQLNULL,
	                          ScalarFunction::UnaryFunction<string_t, int32_t, ErrorOperator>);
	// Set the function with side effects to avoid the optimization.
	fun.stability = FunctionStability::VOLATILE;
	BaseScalarFunction::SetReturnsError(fun);
	return fun;
}

} // namespace duckdb
