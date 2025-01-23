const express = require("express");
const {
  handleFingerprint,
} = require("../controllers/fingerprintController.js");
const router = express.Router();

router.post("/", handleFingerprint);

module.exports = router;
