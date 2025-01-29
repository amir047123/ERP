const mongoose = require("mongoose");

const UserSchema = new mongoose.Schema({
  name: { type: String, default: "Unknown User" },
  fingerprintHash: { type: String, required: true, unique: true }, // ✅ Ensure we're using `fingerprintHash`
  mistId: { type: String, unique: true, required: true },
});

module.exports = mongoose.model("User", UserSchema);
