-- TODO(ddiez): Migrate all tests to a proper unit test using Dremel
-- minicluster
-- source ./dates_udf.sql;

SET @epoch_date = "1970-01-01";
SET @epoch_dateid = 19700101;
SET @epoch_timestamp = TIMESTAMP "1970-01-01 00:00:00 UTC";
SET @epoch_timestamp_string = "1970-01-01 00:00:00+00";
SET @epoch_usec = 0;
SET @epoch_unixdate = 0;
-- TODO(ddiez): Line break this param once it's allowed.
SET @sample_pst_ts = [TIMESTAMP("2016-12-31 00:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 01:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 01:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 02:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 03:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 04:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 05:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 06:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 07:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 08:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 09:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 10:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 11:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 12:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 13:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 14:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 15:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 16:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 17:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 18:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 19:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 20:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 21:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 22:00:00", "America/Los_Angeles"), TIMESTAMP("2016-12-31 23:00:00", "America/Los_Angeles")];
SET @sample_pst_date = DATE "2016-12-31";
SET @sample_pst_week_start = DATE "2016-12-25";

-- Roundtrip between date assumed in PST and timestamp
DEFINE INLINE TABLE rt_date_ts
SELECT
  epoch_date,
  epoch_date = @epoch_date AS check

FROM
  (SELECT
    TS_TO_DATE(DATE_TO_TS(@epoch_date)) AS epoch_date
  )
;

-- Roundtrip between date assumed in PST and usec
DEFINE INLINE TABLE rt_date_usec
SELECT
  epoch_date,
  epoch_date = @epoch_date AS check

FROM
  (SELECT
    USEC_TO_DATE(DATE_TO_USEC(@epoch_date)) AS epoch_date
  )
;

-- Roundtrip between date assumed in PST and sec
DEFINE INLINE TABLE rt_date_sec
SELECT
  epoch_date,
  epoch_date = @epoch_date AS check

FROM
  (SELECT
    SEC_TO_DATE(DATE_TO_SEC(@epoch_date)) AS epoch_date
  )
;

-- Roundtrip between date and dateid
DEFINE INLINE TABLE rt_date_dateid
SELECT
  epoch_date,
  epoch_date = @epoch_date AS check

FROM
  (SELECT
    DATEID_TO_DATE(DATE_TO_DATEID(@epoch_date)) AS epoch_date
  )
;

-- Roundtrip between dateid assumed in PST and timestamp
DEFINE INLINE TABLE rt_dateid_ts
SELECT
  epoch_dateid,
  epoch_dateid = @epoch_dateid AS check

FROM
  (SELECT
    TS_TO_DATEID(DATEID_TO_TS(@epoch_dateid)) AS epoch_dateid
  )
;

-- Roundtrip between dateid assumed in PST and usec
DEFINE INLINE TABLE rt_dateid_usec
SELECT
  epoch_dateid,
  epoch_dateid = @epoch_dateid AS check

FROM
  (SELECT
    USEC_TO_DATEID(DATEID_TO_USEC(@epoch_dateid)) AS epoch_dateid
  )
;

-- Roundtrip between dateid assumed in PST and sec
DEFINE INLINE TABLE rt_dateid_sec
SELECT
  epoch_dateid,
  epoch_dateid = @epoch_dateid AS check

FROM
  (SELECT
    SEC_TO_DATEID(DATEID_TO_SEC(@epoch_dateid)) AS epoch_dateid
  )
;

-- Roundtrip between timestamp and usec
DEFINE INLINE TABLE rt_ts_usec
SELECT
  epoch_timestamp,
  TS_TO_STRING(epoch_timestamp) = @epoch_timestamp_string AS check

FROM
  (SELECT
    USEC_TO_TS(TS_TO_USEC(@epoch_timestamp)) AS epoch_timestamp
  )
;

-- Roundtrip between timestamp and sec
DEFINE INLINE TABLE rt_ts_sec
SELECT
  epoch_timestamp,
  TS_TO_STRING(epoch_timestamp) = @epoch_timestamp_string AS check

FROM
  (SELECT
    SEC_TO_TS(TS_TO_SEC(@epoch_timestamp)) AS epoch_timestamp
  )
;

-- Roundtrip between unixdate and date
DEFINE INLINE TABLE rt_unixdate_date
SELECT
  epoch_unixdate,
  epoch_unixdate = @epoch_unixdate AS check

FROM
  (SELECT
    DATE_TO_UNIXDATE(UNIXDATE_TO_DATE(@epoch_unixdate)) AS epoch_unixdate
  )
;

-- Roundtrip between unixdate and dateid
DEFINE INLINE TABLE rt_unixdate_dateid
SELECT
  epoch_unixdate,
  epoch_unixdate = @epoch_unixdate AS check

FROM
  (SELECT
    DATEID_TO_UNIXDATE(UNIXDATE_TO_DATEID(@epoch_unixdate)) AS epoch_unixdate
  )
;

-- Roundtrip between unixdate and timestamp
DEFINE INLINE TABLE rt_unixdate_ts
SELECT
  epoch_unixdate,
  epoch_unixdate = @epoch_unixdate AS check

FROM
  (SELECT
    TS_TO_UNIXDATE(UNIXDATE_TO_TS(@epoch_unixdate)) AS epoch_unixdate
  )
;

-- Roundtrip between unixdate and usec
DEFINE INLINE TABLE rt_unixdate_usec
SELECT
  epoch_unixdate,
  epoch_unixdate = @epoch_unixdate AS check

FROM
  (SELECT
    USEC_TO_UNIXDATE(UNIXDATE_TO_USEC(@epoch_unixdate)) AS epoch_unixdate
  )
;

-- Roundtrip between unixdate and sec
DEFINE INLINE TABLE rt_unixdate_sec
SELECT
  epoch_unixdate,
  epoch_unixdate = @epoch_unixdate AS check

FROM
  (SELECT
    SEC_TO_UNIXDATE(UNIXDATE_TO_SEC(@epoch_unixdate)) AS epoch_unixdate
  )
;

-- Roundtrip between usec and sec
DEFINE INLINE TABLE rt_usec_sec
SELECT
  epoch_usec,
  epoch_usec = @epoch_usec AS check

FROM
  (SELECT
    SEC_TO_USEC(USEC_TO_SEC(@epoch_usec)) AS epoch_usec
  )
;

-- Epoch timestamp should translate to microseconds 0
DEFINE INLINE TABLE epoch_zero
SELECT
  epoch_usec,
  epoch_usec = @epoch_usec AS check

FROM
  (SELECT
    TS_TO_USEC(@epoch_timestamp) AS epoch_usec
  )
;

-- PST timestamps should always return the same date.
DEFINE INLINE TABLE day_ts_date
SELECT
  ts_utc,
  pst_date,
  pst_date = @sample_pst_date AS check

FROM
  (SELECT
    ts_utc,
    TS_TO_DATE(ts_utc) AS pst_date
  FROM UNNEST(@sample_pst_ts) AS ts_utc
  )
;

-- A PST day that spans between 2 weeks in UTC should only return the same
-- start-of-week date.
DEFINE INLINE TABLE week_start_pst
SELECT
  ts_utc,
  week_start_pst,
  week_start_pst = @sample_pst_week_start AS check

FROM
  (SELECT
  ts_utc,
  WEEK_START(TS_TO_USEC(ts_utc)) as week_start_pst

  FROM UNNEST(@sample_pst_ts) AS ts_utc
  )
;

-- Run all checks and complile the results
DEFINE MACRO EXPECT_TRUE
SELECT
  "$1" AS check,
  LOGICAL_AND(check) AS pass
FROM $1
;

-- Execute this query and make sure all tests return true
SELECT
  check,
  pass

FROM
  ($EXPECT_TRUE(rt_date_ts))
UNION ALL
  ($EXPECT_TRUE(rt_date_usec))
UNION ALL
  ($EXPECT_TRUE(rt_date_sec))
UNION ALL
  ($EXPECT_TRUE(rt_date_dateid))
UNION ALL
  ($EXPECT_TRUE(rt_dateid_ts))
UNION ALL
  ($EXPECT_TRUE(rt_dateid_usec))
UNION ALL
  ($EXPECT_TRUE(rt_dateid_sec))
UNION ALL
  ($EXPECT_TRUE(rt_ts_usec))
UNION ALL
  ($EXPECT_TRUE(rt_ts_sec))
UNION ALL
  ($EXPECT_TRUE(rt_unixdate_date))
UNION ALL
  ($EXPECT_TRUE(rt_unixdate_dateid))
UNION ALL
  ($EXPECT_TRUE(rt_unixdate_ts))
UNION ALL
  ($EXPECT_TRUE(rt_unixdate_usec))
UNION ALL
  ($EXPECT_TRUE(rt_unixdate_sec))
UNION ALL
  ($EXPECT_TRUE(rt_usec_sec))
UNION ALL
  ($EXPECT_TRUE(epoch_zero))
UNION ALL
  ($EXPECT_TRUE(day_ts_date))
UNION ALL
  ($EXPECT_TRUE(week_start_pst))
