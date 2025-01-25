const mongoose = require("mongoose");

const UserSchema = new mongoose.Schema({
  name: { type: String, default: "Unknown User" },
  rawFingerprintImage: { type: String, required: true, unique: true },
  mistId: { type: String, unique: true, required: true },
});

module.exports = mongoose.model("User", UserSchema);
