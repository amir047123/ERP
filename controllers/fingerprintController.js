const {
  createUser,
  findUserByFingerprintImage,
  getUserCount,
} = require("../services/userService");

const handleFingerprint = async (req, res) => {
  try {
    const { rawFingerprintImage } = req.body;

    if (!rawFingerprintImage) {
      console.log("No fingerprint image provided in the request.");
      return res.status(400).json({ message: "No fingerprint image provided" });
    }

    console.log("Checking for existing fingerprint in the database...");
    const existingUser = await findUserByFingerprintImage(rawFingerprintImage);

    if (existingUser) {
      console.log(`Fingerprint matched: ID ${existingUser.mistId}`);
      return res.status(200).json({
        message: `Match ID ${existingUser.mistId}`,
        user: existingUser,
      });
    } else {
      console.log("No matching fingerprint found. Creating new user...");

      // Generate MIST serial ID
      const userCount = await getUserCount();
      const mistId = `MIST-${String(userCount + 1).padStart(2, "0")}`;
      console.log(`Generated MIST ID: ${mistId}`);

      // Create a new user
      const newUser = await createUser({
        name: "Unknown User",
        rawFingerprintImage,
        mistId,
      });

      console.log("New user created successfully:", newUser);
      return res.status(201).json({
        message: `created ID ${newUser.mistId}`,
        user: newUser,
      });
    }
  } catch (error) {
    console.error("Error handling fingerprint:", error.message);

    if (error.code === 11000) {
      return res
        .status(400)
        .json({ message: "Duplicate fingerprint detected" });
    }

    res.status(500).json({ error: "An internal server error occurred" });
  }
};

module.exports = { handleFingerprint };
