-- @author prabhd 
-- @created 2012-12-05 12:00:00 
-- @modified 2012-12-05 12:00:00 
-- @tags dml 
-- @db_name dmldb
-- @description update_test7: Update distcol where join on target table non dist key
\echo --start_ignore
set gp_enable_column_oriented_table=on;
\echo --end_ignore
SELECT SUM(a) FROM dml_heap_r WHERE a = 1;
UPDATE dml_heap_r SET a = dml_heap_r.a + 1 FROM dml_heap_s WHERE dml_heap_r.a = dml_heap_s.a;
SELECT SUM(a) FROM dml_heap_r WHERE a = 1;
