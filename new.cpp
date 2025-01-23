#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <time.h>

// WiFi credentials
const char* ssid = "Mangrove IT";
const char* password = "mangroveIT@2023";

// Backend API URL
const char* apiURL = "https://erp-orpin-mu.vercel.app/api/fingerprint";

// Fingerprint sensor setup
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

// Global serial number for unique name generation
static int serialNumber = 1;

// Function Prototypes
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

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  initializeNTP();

  // Initialize fingerprint sensor
  finger.begin(57600);
  Serial.println("Initializing fingerprint sensor...");

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor initialized successfully!");
  } else {
    Serial.println("Failed to initialize fingerprint sensor. Please check wiring or power.");
    while (1);
  }
}

// Main loop function
void loop() {
  Serial.println("Place your finger on the sensor...");
  int p = finger.getImage();

  if (p != FINGERPRINT_OK) {
    Serial.println("No finger detected or image failed.");
    delay(2000);
    return;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image to template.");
    delay(2000);
    return;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Fingerprint matched! ID: ");
    Serial.println(finger.fingerID);
    handleFingerprint(finger.fingerID, "match"); // Match mode
  } else {
    Serial.println("Fingerprint not found in database. Proceeding with registration...");

    String name = generateUniqueName();
    Serial.print("Registering user: ");
    Serial.println(name);

    int id = enrollFingerprint();
    if (id > 0) {
      Serial.print("Fingerprint enrolled successfully with ID: ");
      Serial.println(id);
      handleFingerprint(id, "register", name); // Register mode
    } else {
      Serial.println("Failed to enroll fingerprint.");
    }
  }

  delay(5000);
}

// Function to enroll a fingerprint
int enrollFingerprint() {
  int id = findAvailableID();
  Serial.print("Enrolling fingerprint ID #");
  Serial.println(id);

  int p = -1;
  Serial.println("Place your finger on the sensor...");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.print(".");
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
      Serial.print(".");
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

    const int maxRetries = 3;
    int attempt = 0;
    bool success = false;

    while (attempt < maxRetries && !success) {
      int httpResponseCode = http.POST(payload);
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);

        if (httpResponseCode == 200 || httpResponseCode == 201) {
          success = true;
          Serial.println("Operation successful.");
        } else {
          Serial.println("Server returned an error. Retrying...");
        }
      } else {
        Serial.print("HTTP POST failed, error: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
      }

      attempt++;
      delay(2000 * attempt); // Exponential backoff
    }

    if (!success) {
      Serial.println("Failed to communicate with the server after multiple attempts.");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}
