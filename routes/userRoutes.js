const express = require("express");
const {
  registerUser,
  getUserByFingerprint,
} = require("../controllers/userController");
const { handleFingerprint } = require("../controllers/fingerprintController");

const router = express.Router();

router.post("/register", registerUser);
router.get("/:fingerprintId", getUserByFingerprint);
router.post("/fingerprint", handleFingerprint);

module.exports = router;
