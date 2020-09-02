

CREATE TEMPORARY TABLE FUNCTION LookUpIm(
  Dat TABLE<rating STRING>,
  IndividualMultiplier TABLE<rating STRING, individual_multiplier DOUBLE>,
  foo STRING,
  bar STRING
) RETURNS TABLE<person_id INT64, individual_multiplier DOUBLE>;
