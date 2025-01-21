const express = require("express");
const {
  logAttendanceByFingerprint,
} = require("../controllers/attendanceController");
const router = express.Router();

router.post("/", logAttendanceByFingerprint);

module.exports = router;
