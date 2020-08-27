SELECT column_alias
FROM
  (
    WITH
      table_one AS (
        SELECT
          'A string that is long enough to break the query into two lines'
            AS column_alias
      )
    SELECT *
  );
