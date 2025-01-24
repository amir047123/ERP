const mongoose = require("mongoose");

const UserSchema = new mongoose.Schema({
  fingerprintId: { type: Number },
  name: { type: String },
  fingerImage: {
    type: String,
  },
});

module.exports = mongoose.model("User", UserSchema);
