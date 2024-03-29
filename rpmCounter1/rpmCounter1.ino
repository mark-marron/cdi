#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <Base64.h>

const int analogInputPin = A0;
const int sparkPin = 2;
int count = 0;
bool status = false;
const int chipSelect = 4;

unsigned long previousMillis = 0;
const long interval = 2000;

// engine properties
int voltage_threshold = 400;
int number_of_cylinders = 2;
int max_rpm = 6000;

// safety properties
int level = 2;
int hour_levels[2] = {5, 10};
int hours = 4;

void setup() {
  Serial.begin(9600);
  pinMode(sparkPin, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed");
    return;
  }
  File engine_profile = SD.open("ENGINE~1.JSO");
  File safety_profile = SD.open("SAFETY.PRO");
  if (!engine_profile || !safety_profile) {
    Serial.println("Failed to read profiles");
    return;
  }

  const int maxSize = 100;
  char encoded_safety_profile[maxSize];
  int i = 0;
  while (safety_profile.available() && i < maxSize - 1) {
    char c = safety_profile.read();
    encoded_safety_profile[i] = c;
    i++;
  }
  encoded_safety_profile[i] = "\0";
  safety_profile.close();

  Serial.println(encoded_safety_profile);
  char encodedData = encoded_safety_profile;
  int encodedDataLength = strlen(encoded_safety_profile);
  int decodedDataLength = Base64.decodedLength(encodedData, encodedDataLength);
  char decodedData[decodedDataLength + 1];
  Base64.decode(decodedData, encoded_safety_profile, encodedDataLength);
  Serial.println(decodedData);
  String decodedSafety

  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 30;
  DynamicJsonDocument jsonBuffer(bufferSize);
  DeserializationError error = deserializeJson(jsonBuffer, engine_profile);
  if (error) {
    Serial.println("Failed to parse engine profile JSON");
    return;
  }
  JsonObject engine = jsonBuffer.as<JsonObject>();

  DynamicJsonDocument decodedJsonBuffer(bufferSize);
  DeserializationError decodedError = deserializeJson(decodedJsonBuffer, decodedData);
  if (decodedError) {
    Serial.println("Failed to parse safety profile JSON");
    return;
  }
  level = decodedJsonBuffer["level"];

  voltage_threshold = engine["voltage_threshold"];
  number_of_cylinders = engine["number_of_cylinders"];
  max_rpm = engine["max_rpm"];

  Serial.println(voltage_threshold);
  Serial.println(number_of_cylinders);
  Serial.println(max_rpm);
  Serial.println(level);
}

void loop() {

  int sensorVal = analogRead(analogInputPin);
  unsigned long currentMillis = millis();
  
  if (!status && sensorVal >= 500) {
    count++;
    spark();
    status = true;
  }
  if (sensorVal <= 400) {
    status = false;
    digitalWrite(sparkPin, HIGH);
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // Serial.print("RPM: ");
    // Serial.println(count * 30);
    count = 0;
  }
}

void spark() {
  digitalWrite(sparkPin, LOW);
}
