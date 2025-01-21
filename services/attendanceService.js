const Attendance = require("../models/ attendance.model");

const logAttendance = async (attendanceData) => {
  const attendance = new Attendance(attendanceData);
  return await attendance.save();
};

const getAttendanceLogs = async () => {
  return await Attendance.find().sort({ timestamp: -1 });
};

module.exports = { logAttendance, getAttendanceLogs };
