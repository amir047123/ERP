const User = require("../models/user.model");

const findUserByFingerprint = async (fingerprintId) => {
  return await User.findOne({ fingerprintId });
};

const createUser = async (userData) => {
  const user = new User(userData);
  return await user.save();
};

module.exports = { findUserByFingerprint, createUser };
