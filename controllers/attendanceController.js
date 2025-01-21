const { logAttendance } = require("../services/attendanceService");
const { findUserByFingerprint } = require("../services/userService");

const logAttendanceByFingerprint = async (req, res) => {
  try {
    const { fingerprintId } = req.body;
    const user = await findUserByFingerprint(fingerprintId);

    if (!user) return res.status(404).json({ message: "User not found" });

    const attendance = await logAttendance({
      fingerprintId: user.fingerprintId,
      name: user.name,
    });

    res
      .status(200)
      .json({ message: "Attendance logged successfully", attendance });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

module.exports = { logAttendanceByFingerprint };
