const express = require("express");
const {
  registerUser,
  getUserByFingerprint,
} = require("../controllers/userController");
const router = express.Router();

router.post("/register", registerUser);
router.get("/:fingerprintId", getUserByFingerprint);

module.exports = router;
