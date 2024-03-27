#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>

const int analogInputPin = A0;
const int sparkPin = 2;
int count = 0;
bool status = false;
const int chipSelect = 8;

unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(sparkPin, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed");
    return;
  }

  File engine_profile = SD.open("ENGINE~1.JSO");

  if (!engine_profile) {
    Serial.println("Failed to read file");
    return;
  }

  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 30;
  DynamicJsonDocument jsonBuffer(bufferSize);
  DeserializationError error = deserializeJson(jsonBuffer, engine_profile);

  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  JsonObject engine = jsonBuffer.as<JsonObject>();
  int threshold = engine["volatge_threshold"];
  int cylinders = engine["number_of_cylinders"];

  Serial.println("Data from JSON file:");
  Serial.print("Voltage Threshold: ");
  Serial.println(threshold);
  Serial.print("Number of Cylinders: ");
  Serial.println(cylinders);
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
    Serial.print("RPM: ");
    Serial.println(count * 30);
    count = 0;
  }
}

void spark() {
  digitalWrite(sparkPin, LOW);
}
