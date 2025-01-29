const User = require("../models/user.model");

// ✅ Find user by fingerprint hash
const findUserByFingerprintHash = async (fingerprintHash) => {
  return await User.findOne({ fingerprintHash }); // ✅ Fix field name
};

// ✅ Create a new user
const createUser = async (userData) => {
  const user = new User(userData);
  return await user.save();
};

// ✅ Get total user count
const getUserCount = async () => {
  return await User.countDocuments();
};

module.exports = {
  findUserByFingerprintHash,
  createUser,
  getUserCount,
};
