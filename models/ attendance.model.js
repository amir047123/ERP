const mongoose = require("mongoose");

const AttendanceSchema = new mongoose.Schema({
  fingerprintId: { type: Number, required: true },
  name: { type: String, required: true },
  timestamp: { type: Date, default: Date.now },
});

module.exports = mongoose.model("Attendance", AttendanceSchema);
