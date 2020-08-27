WITH
  table_one AS (
    SELECT 1 AS a_very_very_long_column_name_consisting_of_several_words
  ),
  table_two AS (
    SELECT 2 AS a_very_very_long_column_name_consisting_of_several_words
  )
SELECT a_very_very_long_column_name_consisting_of_several_words
FROM table_one
UNION ALL
  SELECT a_very_very_long_column_name_consisting_of_several_words
  FROM table_two;
