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

    // Add logic to capture the fingerprint image
    if (finger.getImage() == FINGERPRINT_OK) {
      uint8_t imageBuffer[512]; // Buffer for storing fingerprint image data
      int imageSize = finger.getImageData(imageBuffer);

      if (imageSize > 0) {
        // Convert image data to Base64
        String imageBase64 = base64::encode(imageBuffer, imageSize);

        // Add Base64-encoded image to the JSON payload
        doc["image"] = imageBase64;
      } else {
        Serial.println("Failed to read fingerprint image.");
      }
    } else {
      Serial.println("No fingerprint image captured.");
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