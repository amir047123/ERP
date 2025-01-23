const {
  createUser,
  findUserByFingerprint,
} = require("../services/userService");

const handleFingerprint = async (req, res) => {
  try {
    console.log(req.body);

    console.log(req.body.fingerprintId);
    console.log(req.body.name);

    const { mode, fingerprintId, name } = req.body;

    if (mode === "register") {
      // Check if the fingerprint already exists
      const existingUser = await findUserByFingerprint(fingerprintId);
      if (existingUser) {
        return res.status(400).json({ message: "User already exists" });
      }

      // Register the new fingerprint
      const user = await createUser({ fingerprintId, name });
      return res
        .status(201)
        .json({ message: "User registered successfully", user });
    } else if (mode === "match") {
      // Match the fingerprint
      const user = await findUserByFingerprint(fingerprintId);
      if (!user) {
        return res.status(404).json({ message: "User not found" });
      }
      return res.status(200).json({ message: "Fingerprint matched", user });
    } else {
      return res.status(400).json({ message: "Invalid mode specified" });
    }
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

module.exports = { handleFingerprint };
