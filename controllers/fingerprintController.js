const crypto = require("crypto");
const {
  createUser,
  findUserByFingerprintHash,
  getUserCount,
} = require("../services/userService");

const handleFingerprint = async (req, res) => {
  try {
    const { rawFingerprintImage } = req.body;

    if (!rawFingerprintImage) {
      console.log("No fingerprint image provided in the request.");
      return res.status(400).json({ message: "No fingerprint image provided" });
    }

    // ✅ Generate SHA256 Hash of Fingerprint Image
    const fingerprintHash = crypto
      .createHash("sha256")
      .update(rawFingerprintImage)
      .digest("hex");

    console.log("Generated Fingerprint Hash:", fingerprintHash);

    console.log("Checking for existing fingerprint in the database...");
    const existingUser = await findUserByFingerprintHash(fingerprintHash);

    if (existingUser) {
      console.log(`Fingerprint matched: ID ${existingUser.mistId}`);
      return res.status(200).json({
        message: `Match ID ${existingUser.mistId}`,
        user: existingUser,
      });
    } else {
      console.log("No matching fingerprint found. Creating new user...");

      // ✅ Generate Unique MIST ID
      const userCount = await getUserCount();
      const mistId = `MIST-${String(userCount + 1).padStart(2, "0")}`;
      console.log(`Generated MIST ID: ${mistId}`);

      // ✅ Save fingerprintHash instead of rawFingerprintImage
      const newUser = await createUser({
        name: "Unknown User",
        fingerprintHash, // ✅ Store hashed fingerprint, not raw image
        mistId,
      });

      console.log("New user created successfully:", newUser);
      return res.status(201).json({
        message: `Created ID ${newUser.mistId}`,
        user: newUser,
      });
    }
  } catch (error) {
    console.error("❌ ERROR HANDLING FINGERPRINT:", error);
    if (error.code === 11000) {
      return res
        .status(400)
        .json({ message: "Duplicate fingerprint detected" });
    }
    res.status(500).json({ error: error.message });
  }
};

module.exports = { handleFingerprint };
