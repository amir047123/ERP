#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <time.h>
#include <U8g2lib.h>

// WiFi credentials
const char* ssid = "Squad 06";
const char* password = "yarasfm@2026";

// Backend API URL
const char* apiURL = "https://erp-orpin-mu.vercel.app/api/fingerprint";

// Fingerprint sensor setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

// Global serial number for unique name generation
static int serialNumber = 1;

// OLED display setup
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Function prototypes
void handleFingerprint(int fingerprintId, String mode, String name = "");
int enrollFingerprint();
int findAvailableID();
String generateUniqueName();
void initializeNTP();

// Initialize NTP for time synchronization
void initializeNTP() {
  configTime(0, 0, "pool.ntp.org");
  Serial.println("Waiting for NTP time sync...");
  while (!time(nullptr)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nNTP time synchronized.");
}

// Generate a unique user name using the current time and a serial number
String generateUniqueName() {
  time_t now = time(nullptr);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", localtime(&now));
  String uniqueName = "User_" + String(timestamp) + "_" + String(serialNumber);
  serialNumber++;
  return uniqueName;
}

// Arduino setup function
void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);

  // OLED initialization
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.drawStr(0, 10, "Welcome to");
  u8g2.drawStr(0, 25, "Fingerprint System");
  u8g2.drawStr(0, 45, "Initializing...");
  u8g2.sendBuffer();
  delay(3000);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Connecting to WiFi...");
  u8g2.sendBuffer();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "WiFi Connected!");
  u8g2.drawStr(0, 25, "System Ready.");
  u8g2.sendBuffer();
  delay(2000);

  initializeNTP();

  // Initialize fingerprint sensor
  finger.begin(57600);
  Serial.println("Initializing fingerprint sensor...");

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Initializing Sensor...");
  u8g2.sendBuffer();

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor initialized successfully!");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Sensor Ready!");
    u8g2.sendBuffer();
  } else {
    Serial.println("Failed to initialize fingerprint sensor. Please check wiring or power.");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Sensor Error!");
    u8g2.sendBuffer();
    while (1);
  }
}

// Main loop function
void loop() {
  Serial.println("Place your finger on the sensor...");
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Place Finger...");
  u8g2.sendBuffer();

  int p = finger.getImage();

  if (p != FINGERPRINT_OK) {
    Serial.println("No finger detected or image failed.");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "No Finger Detected");
    u8g2.sendBuffer();
    delay(2000);
    return;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image to template.");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Template Error!");
    u8g2.sendBuffer();
    delay(2000);
    return;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint matched! ID: ");
    Serial.println(finger.fingerID);
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Fingerprint Matched!");
    u8g2.setCursor(0, 30);
    u8g2.print("ID: ");
    u8g2.print(finger.fingerID);
    u8g2.sendBuffer();

    // Match mode
    handleFingerprint(finger.fingerID, "match");
  } else {
    Serial.println("Fingerprint not found in database. Proceeding with registration...");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Fingerprint Not Found");
    u8g2.drawStr(0, 30, "Registering...");
    u8g2.sendBuffer();

    String name = generateUniqueName();
    Serial.print("Registering user: ");
    Serial.println(name);

    int id = enrollFingerprint();
    if (id > 0) {
      Serial.print("Fingerprint enrolled successfully with ID: ");
      Serial.println(id);
      u8g2.clearBuffer();
      u8g2.drawStr(0, 10, "Enrolled Successfully!");
      u8g2.setCursor(0, 30);
      u8g2.print("ID: ");
      u8g2.print(id);
      u8g2.sendBuffer();

      // Register mode
      handleFingerprint(id, "register", name);
    } else {
      Serial.println("Failed to enroll fingerprint.");
      u8g2.clearBuffer();
      u8g2.drawStr(0, 10, "Enroll Failed!");
      u8g2.sendBuffer();
    }
  }

  delay(5000);
}

// Function to enroll a fingerprint
int enrollFingerprint() {
  int id = findAvailableID();
  if (id < 0) {
    return -1;
  }

  Serial.print("Enrolling fingerprint ID #");
  Serial.println(id);

  int p = -1;
  Serial.println("Place your finger on the sensor...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      delay(500);
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Failed to capture fingerprint image. Try again.");
      return -1;
    }
  }

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert fingerprint image to template.");
    return -1;
  }

  Serial.println("Remove your finger.");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  Serial.println("Place the same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      delay(500);
    } else if (p == FINGERPRINT_IMAGEFAIL) {
      Serial.println("Failed to capture fingerprint image. Try again.");
      return -1;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert second fingerprint image to template.");
    return -1;
  }

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints do not match. Try again.");
    return -1;
  }

  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to store fingerprint model. Try again.");
    return -1;
  }

  return id;
}

// Find the first available ID for a fingerprint
int findAvailableID() {
  for (int i = 1; i <= 127; i++) {
    if (finger.loadModel(i) != FINGERPRINT_OK) {
      return i;
    }
  }
  Serial.println("No available fingerprint IDs. Sensor memory is full.");
  return -1;
}

// Unified function to handle fingerprint registration and matching
void handleFingerprint(int fingerprintId, String mode, String name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiURL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["mode"] = mode;
    doc["fingerprintId"] = fingerprintId;
    if (mode == "register") {
      doc["name"] = name;
    }

    String payload;
    serializeJson(doc, payload);

    Serial.print("Payload Sent: ");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);

      StaticJsonDocument<200> responseDoc;
      deserializeJson(responseDoc, response);

      if (responseDoc.containsKey("user")) {
        const char* name = responseDoc["user"]["name"];
        int id = responseDoc["user"]["fingerprintId"];

        // Display user details on the OLED
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "User Found!");
        u8g2.setCursor(0, 30);
        u8g2.print("Name: ");
        u8g2.print(name);
        u8g2.setCursor(0, 50);
        u8g2.print("ID: ");
        u8g2.print(id);
        u8g2.sendBuffer();
      }
    } else {
      Serial.print("HTTP POST failed, error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}
