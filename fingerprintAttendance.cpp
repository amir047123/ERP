#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <time.h>

// WiFi
const char* ssid = "Squad 06";        
const char* password = "yarasfm@2026"; 

// Backend API URLs
const char* registerURL = "https://attendance-one-omega.vercel.app/api/users/register"; 
const char* userLookupURL = "https://attendance-one-omega.vercel.app/api/users";        

HardwareSerial mySerial(2); 
Adafruit_Fingerprint finger(&mySerial);

static int serialNumber = 1;

void initializeNTP() {
  configTime(0, 0, "pool.ntp.org"); 
  Serial.println("Waiting for NTP time sync...");
  while (!time(nullptr)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nNTP time synchronized.");
}

String generateUniqueName() {
  time_t now = time(nullptr); 
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", localtime(&now)); 
  String uniqueName = "User_" + String(timestamp) + "_" + String(serialNumber);
  serialNumber++;
  return uniqueName;
}

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

  finger.begin(57600); 
  Serial.println("Initializing fingerprint sensor...");

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor initialized successfully!");
  } else {
    Serial.println("Failed to initialize fingerprint sensor. Please check wiring or power.");
    while (1); 
  }
}

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

    fetchUserDetails(finger.fingerID);
  } else {
    Serial.println("Fingerprint not found in database. Proceeding with registration...");

    String name = generateUniqueName();
    Serial.print("Registering user: ");
    Serial.println(name);

    int id = enrollFingerprint();
    if (id > 0) {
      Serial.print("Fingerprint enrolled successfully with ID: ");
      Serial.println(id);

      if (registerFingerprint(id, name)) {
        Serial.println("User registered successfully!");
      } else {
        Serial.println("Failed to register user.");
      }
    } else {
      Serial.println("Failed to enroll fingerprint.");
    }
  }

  delay(5000); 
}

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

int findAvailableID() {
  for (int i = 1; i <= 127; i++) { 
    if (finger.loadModel(i) != FINGERPRINT_OK) {
      return i; 
    }
  }
  Serial.println("No available fingerprint IDs. Sensor memory is full.");
  return -1; 
}

bool registerFingerprint(int id, String name) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(registerURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"fingerprintId\": " + String(id) + ", \"name\": \"" + name + "\"}";
    Serial.print("Sending payload: ");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      if (httpResponseCode == 201) { 
        Serial.println("User registered successfully!");
        http.end();
        return true; 
      } else if (httpResponseCode == 400) { 
        Serial.println("Error: User already exists.");
      } else {
        Serial.println("Error: Unexpected response from server.");
      }
    } else {
      Serial.print("HTTP POST failed, error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
  return false; 
}

void fetchUserDetails(int fingerprintId) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(userLookupURL) + "/" + String(fingerprintId); 
    http.begin(url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);

      if (httpResponseCode == 200) {
        String payload = http.getString();
        Serial.print("User Data: ");
        Serial.println(payload); 
      } else if (httpResponseCode == 404) {
        Serial.println("User not found.");
      } else {
        Serial.println("Unexpected response from server.");
      }
    } else {
      Serial.print("HTTP GET failed, error: ");
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("WiFi not connected.");
  }
}
