#include <SPI.h>
#include <SD.h>
#include <Base64.h>
#include <LiquidCrystal.h>

bool verbose = true;

const int analogInputPin = A0;
const int sparkPin = 10;
int count = 0;
bool status = false;
const int chipSelect = 8;
unsigned long previousMillis = 0;

// LCD screen
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// interval for how frequently to calculate rpm
const long interval = 2000;

// interval for how frequently to write runtime to safety profile
const int write_interval = 60000;

// engine properties
int voltage_threshold = 400;
int number_of_cylinders = 2;
int factory_max_rpm = 6000;

// safety properties
int level = 2;
int level_requirements[2] = {5, 10};
int seconds = 3600;

int max_rpm = 0;
volatile unsigned long current_rpm = 0;
int runtime = 0;

unsigned long lastPulse = 0;
unsigned long currentPulse = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("RPM: ");

  pinMode(sparkPin, OUTPUT);
  digitalWrite(sparkPin, HIGH);

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
    factory_max_rpm =  engine_profile.parseInt();
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

  // calculate level
  int size = sizeof(level_requirements) / sizeof(level_requirements[0]);
  level = 1;
  for (int i=0; i<=size; i++) {
    if (seconds > level_requirements[i]) {
      level ++;
    }
  }

  // calculate maxRPM
  max_rpm = 3000 + (level/3.0f) * (factory_max_rpm - 3000);

  if (verbose) {
    Serial.println("Engine Info:");
    Serial.println(voltage_threshold);
    Serial.println(number_of_cylinders);
    Serial.println(factory_max_rpm);
    Serial.println("Safety Info:");
    Serial.println(seconds);
    Serial.print("{");
    for (int i=0; i<=sizeof(level_requirements) / sizeof(level_requirements[0]); i++) {
      Serial.print(level_requirements[i]);
      Serial.print(" ");
    }
    Serial.println("}");
    Serial.println("===============");
    Serial.print("Level: ");
    Serial.println(level);
    Serial.print("Max RPM: ");
    Serial.println(max_rpm);
  }
}

void loop() {

  int sensorVal = analogRead(analogInputPin);
  unsigned long currentMillis = millis();

  if (!status && sensorVal >= 500) {
    count++;
    lastPulse = currentPulse;
    currentPulse = millis();

    spark();
    status = true;
  }
  if (sensorVal <= 400) {
    status = false;
    digitalWrite(sparkPin, HIGH);
    // currentPulse = lastPulse;
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.print(currentPulse);
    Serial.print(" - ");
    Serial.print(lastPulse);
    Serial.print(" = ");
    Serial.println(currentPulse - lastPulse);
    Serial.print("RPM: ");
    Serial.println(60000/(currentPulse-lastPulse));
  }
}

void spark() {
  // Serial.println("Spark");
  digitalWrite(sparkPin, LOW);
}
