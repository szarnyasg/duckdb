#!/bin/bash

set -e

export PATHVAR=/home/szarnyasg/git/snb/ldbc-example-graph/snb-example-graph-csv-composite-merge-foreign-raw
export POSTFIX=_0_0.csv

rm ldbc.duckdb
cat schema-*.sql      | duckdb ldbc.duckdb
cat snb-load.sql      | sed "s|PATHVAR|$PATHVAR|" | sed "s|POSTFIX|$POSTFIX|" | duckdb ldbc.duckdb
cat snb-transform.sql | duckdb ldbc.duckdb
cat snb-export.sql    | duckdb ldbc.duckdb
