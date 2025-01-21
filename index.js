require("dotenv").config();
const express = require("express");
const connectDB = require("./config/db");
const userRoutes = require("./routes/userRoutes");
const attendanceRoutes = require("./routes/attendanceRoutes");

const app = express();

app.use(express.json());

connectDB();

app.get("/", (req, res) => {
  res.send("Server is running successfully!");
});

app.use("/api/users", userRoutes);
app.use("/api/attendance", attendanceRoutes);

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));
