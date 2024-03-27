#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
// #include <base64.hpp>
#include <Base64.h>

// Set the CS pin for your SD card module
const int chipSelect = 4;

void setup() {
  Serial.begin(9600);

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed.");
    return;
  }

  Serial.println("SD card initialized.");

  // Open the JSON file
  File file = SD.open("ENGINE.PRO");

  if (!file) {
    Serial.println("Failed to open file.");
    return;
  }

  Serial.println("File opened successfully");

  char encodedData = "";
  while (file.available()) {
    encodedData += (char)file.read();
  }
  file.close();

  int encodedDataLength = strlen(encodedData);
  int decodedDataLength = Base64.decodedLength(encodedData, encodedDataLength);
  char decodedData[decodedDataLength + 1];
  Base64.decode(decodedData, encodedData, encodedDataLength);

  Serial.println(decodedData);


  // Allocate a buffer for the JSON data
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 30;
  DynamicJsonDocument jsonBuffer(bufferSize);

  // Parse the JSON data
  DeserializationError error = deserializeJson(jsonBuffer, decodedData);

  if (error) {
    Serial.println("Failed to parse JSON.");
    return;
  }

  // Access the parsed JSON data
  JsonObject root = jsonBuffer.as<JsonObject>();

  // Access individual fields
  int threshold = root["volatge_threshold"];
  int cylinders = root["number_of_cylinders"];

  // Print the data
  Serial.println("Data from JSON file:");
  Serial.print("Voltage Threshold: ");
  Serial.println(threshold);
  Serial.print("Number of Cylinders: ");
  Serial.println(cylinders);
}

void loop() {
  // Nothing to do here
}

