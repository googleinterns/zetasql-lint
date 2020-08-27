-- NOLINT(consistent-letter-case)
WITH Offset AS (
  SELECT very_very_long_column_name_foo FROM UNNEST(GENERATE_ARRAY(0, 30))
    AS very_very_long_column_name_foo WITH OFFSET AS bar)
  SELECT ANONYMIZATION FROM Offset;
