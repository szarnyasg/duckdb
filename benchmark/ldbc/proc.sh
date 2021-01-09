#!/bin/bash

set -e

export PATHVAR=/home/szarnyasg/git/snb/ldbc-example-graph/snb-example-graph-csv-composite-merge-foreign-raw
export POSTFIX=_0_0.csv

rm -rf ldbc.duckdb
cat schema-*.sql      | ./duckdb ldbc.duckdb
cat snb-load.sql      | sed "s|PATHVAR|$PATHVAR|g" | sed "s|POSTFIX|$POSTFIX|g" | ./duckdb ldbc.duckdb
#cat snb-transform.sql | sed "s|:bulkLoadTime|''|g" | ./duckdb ldbc.duckdb
cat snb-export.sql    | ./duckdb ldbc.duckdb
