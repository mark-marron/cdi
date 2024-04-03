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
int level_requirements[2] = {5, 10};
int seconds = 3600;

void setup() {
  Serial.begin(9600);
  Serial.println("=========================================================");
  pinMode(sparkPin, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed");
    return;
  }
  File engine_profile = SD.open("ENGINE.PRO");
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

  // decode base64 safety profile
  char encodedData = encoded_safety_profile;
  int encodedDataLength = strlen(encoded_safety_profile);
  int decodedDataLength = Base64.decodedLength(encodedData, encodedDataLength);
  char decodedData[decodedDataLength + 1];
  Base64.decode(decodedData, encoded_safety_profile, encodedDataLength);
  String decoded_safety_profile = decodedData;

  // assign engine data to variables
  if (engine_profile.available()) {
    voltage_threshold = engine_profile.parseInt();
    engine_profile.read();
    number_of_cylinders =  engine_profile.parseInt();
    engine_profile.read();
    max_rpm =  engine_profile.parseInt();
  }
  engine_profile.close();

  // assign safety data to variables
  int i1 = decoded_safety_profile.indexOf("\n");
  int i2 = decoded_safety_profile.indexOf("\n", i1 + 1);
  int i3 = decoded_safety_profile.indexOf("\n", i2 + 1);
  int l1 = decoded_safety_profile.substring(0, i1).toInt();
  int l2 = decoded_safety_profile.substring(i1, i2).toInt();
  seconds = decoded_safety_profile.substring(i2, i3).toInt();
  level_requirements[0] = l1;
  level_requirements[1] = l2;

  if (true) {
    Serial.println("Engine Info:");
    Serial.println(voltage_threshold);
    Serial.println(number_of_cylinders);
    Serial.println(max_rpm);
    Serial.println("Safety Info:");
    Serial.println(seconds);
    Serial.print("{");
    for (int i=0; i<=sizeof(level_requirements) / sizeof(level_requirements[0]); i++) {
      Serial.print(level_requirements[i]);
      Serial.print(" ");
    }
    Serial.println("}");
  }

  // calculate level
  int size = sizeof(level_requirements) / sizeof(level_requirements[0]);
  level = 1;
  for (int i=0; i<=size; i++) {
    if (seconds > level_requirements[i]) {
      level ++;
    }
  }

  // calculate maxRPM
  max_rpm = 3000 + (level/3) * (max_rpm - 3000);
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
