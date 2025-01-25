const User = require("../models/user.model");

// Find a user by fingerprint image

// Create a new user
const createUser = async (userData) => {
  const user = new User(userData);
  return await user.save();
};

// Get the total number of users
const getUserCount = async () => {
  return await User.countDocuments();
};
const findUserByMistId = async (mistId) => {
  return await User.findOne({ mistId });
};
const findUserByFingerprintImage = async (rawFingerprintImage) => {
  return await User.findOne({ rawFingerprintImage });
};

module.exports = {
  findUserByFingerprintImage,
  createUser,
  getUserCount,
  findUserByMistId,
};
