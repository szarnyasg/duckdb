DuckDB implementation of the queries from the [LDBC Social Network Benchmark](https://arxiv.org/abs/2001.02299).

Download the data, initialize the schema, and load the data.

## Test load performance using a single LDBC CSV

```bash
python download-benchmark-data.py

# unzip comment CSV
gunzip sf0.1/comment_0_0.csv.gz

# copy
rm ldbc.duckdb
cat schema.sql | duckdb ldbc.duckdb
time echo "COPY comment FROM 'sf0.1/dynamic/comment_0_0.csv' (DELIMITER '|', HEADER, TIMESTAMPFORMAT '%Y-%m-%dT%H:%M:%S.%g+00:00');" | duckdb ldbc.duckdb

# inserts
rm ldbc.duckdb
tail -n +2 sf0.1/dynamic/comment_0_0.csv | awk -F '|' '{ print "INSERT INTO comment VALUES ('\''"substr($1, 1, 23)"'\'', "$2", '\''"$3"'\'', '\''"$4"'\'', '\''"gsub("'\''", "\\'\''", $5)"'\'', "$6", "$7", "$8", 0, 0); " }' > inserts.sql
time cat inserts.sql | duckdb ldbc.duckdb
```
