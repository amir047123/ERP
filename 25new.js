#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <time.h>
#include <U8g2lib.h>
#include <Base64.h> 

// WiFi credentials
const char* ssid = "Squad 06";
const char* password = "yarasfm@2026";

// Backend API URL
const char* apiURL = "https://erp-orpin-mu.vercel.app/api/fingerprint";

// Fingerprint sensor setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

// OLED display setup
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Function prototypes
void sendFingerprintToServer();
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

  Serial.println("Fingerprint captured successfully!");
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Fingerprint Captured!");
  u8g2.sendBuffer();

  // Send fingerprint data to server
  sendFingerprintToServer();

  delay(5000);
}

// Send fingerprint data to the server
// Send fingerprint data to the server
void sendFingerprintToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(apiURL);  // Backend API URL
    http.addHeader("Content-Type", "application/json");

    // Capture fingerprint image
    uint8_t imageBuffer[512];
    int result = finger.getImage();
    if (result != FINGERPRINT_OK) {
      Serial.println("Failed to capture fingerprint image.");
      u8g2.clearBuffer();
      u8g2.drawStr(0, 10, "Capture Error!");
      u8g2.sendBuffer();
      return;
    }

    // Convert fingerprint image to Base64
    String encodedImage = base64::encode(imageBuffer, sizeof(imageBuffer));

    // Create JSON payload
    StaticJsonDocument<1000> doc;
    doc["rawFingerprintImage"] = encodedImage;

    // Serialize JSON document to a String
    String payload;
    serializeJson(doc, payload);

    Serial.println("Sending fingerprint data to server...");
    Serial.println(payload);

    // Send payload to the server
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);

      // Parse JSON response
      StaticJsonDocument<200> responseDoc;
      DeserializationError error = deserializeJson(responseDoc, response);

      if (!error) {
        // Extract the message field
        const char* message = responseDoc["message"];
        Serial.print("Message: ");
        Serial.println(message);

        // Display the message on the OLED
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Server Message:");
        u8g2.drawStr(0, 30, message);
        u8g2.sendBuffer();
      } else {
        Serial.println("Failed to parse JSON response.");
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Parse Error");
        u8g2.sendBuffer();
      }
    } else {
      Serial.print("HTTP POST failed, error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());

      // Display error on OLED
      u8g2.clearBuffer();
      u8g2.drawStr(0, 10, "HTTP Error:");
      u8g2.drawStr(0, 30, http.errorToString(httpResponseCode).c_str());
      u8g2.sendBuffer();
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");

    // Display WiFi error on OLED
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "WiFi Error:");
    u8g2.drawStr(0, 30, "Not Connected");
    u8g2.sendBuffer();
  }
}
