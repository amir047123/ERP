const {
  createUser,
  findUserByFingerprintImage,
  getUserCount,
} = require("../services/userService");

const handleFingerprint = async (req, res) => {
  try {
    const { rawFingerprintImage } = req.body;

    // Validate fingerprint image
    if (!rawFingerprintImage) {
      return res.status(400).json({
        message: "No fingerprint image provided",
      });
    }

    // Check for existing fingerprint
    const existingUser = await findUserByFingerprintImage(rawFingerprintImage);

    if (existingUser) {
      return res.status(200).json({
        message: `Match ID ${existingUser.mistId}`,
        user: existingUser,
      });
    }

    // Create new user
    const userCount = await getUserCount();
    const mistId = `MIST-${String(userCount + 1).padStart(2, "0")}`;
    const newUser = await createUser({
      name: "Unknown User",
      rawFingerprintImage,
      mistId,
    });

    return res.status(201).json({
      message: `created ID ${newUser.mistId}`,
      user: newUser,
    });
  } catch (error) {
    console.error("Error handling fingerprint:", error);

    if (error.code === 11000) {
      return res.status(400).json({
        message: "Duplicate fingerprint detected",
      });
    }

    return res.status(500).json({
      error: "An internal server error occurred",
    });
  }
};

module.exports = { handleFingerprint };
