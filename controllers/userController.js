const {
  createUser,
  findUserByFingerprint,
} = require("../services/userService");

const registerUser = async (req, res) => {
  try {
    const { rawFingerprintImage, name } = req.body;
    console.log("Registration Request:", { rawFingerprintImage, name });

    const existingUser = await findUserByFingerprint(rawFingerprintImage);
    if (existingUser) {
      return res.status(400).json({ message: "User already exists" });
    }

    const user = await createUser({ rawFingerprintImage, name });
    console.log("User Registered Successfully:", user);
    res.status(201).json(user);
  } catch (error) {
    console.error("Registration Error:", error.message);
    res.status(500).json({ error: error.message });
  }
};

const getUserByFingerprint = async (req, res) => {
  try {
    const { rawFingerprintImage } = req.params;
    const user = await findUserByFingerprint(rawFingerprintImage);
    if (!user) return res.status(404).json({ message: "User not found" });
    res
      .status(200)
      .json({ name: user.name, rawFingerprintImage: user.rawFingerprintImage });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
};

module.exports = { registerUser, getUserByFingerprint };
