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

// LCD screen
const int rs = 6, en = 7, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// interval for how frequently to calculate rpm
const long interval = 1000;
unsigned long previousMillis = 0;

// interval for how frequently to write runtime to safety profile
const int write_interval = 3000;
unsigned long previousMillisWrite = 0;
char dataBuffer[100];

File safety_profile;
File engine_profile;
bool profile_status = true;

// engine properties
int voltage_threshold = 400;
int number_of_cylinders = 1;
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

  pinMode(sparkPin, OUTPUT);
  digitalWrite(sparkPin, HIGH);

  if (SD.begin(chipSelect)) {
    engine_profile = SD.open("ENGINE.PRO");
    safety_profile = SD.open("SAFETY.PRO");
  } else {
    profile_status = false;
    return;
  }
  
  if (!engine_profile || !safety_profile) {
    Serial.println("Failed to read profiles");
    profile_status = false;
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
    Serial.println("======Engine Info======");
    Serial.print("Voltage Threshold: ");
    Serial.println(voltage_threshold);
    Serial.print("Number of Cylinders: ");
    Serial.println(number_of_cylinders);
    Serial.print("Factory RPM Limit: ");
    Serial.println(factory_max_rpm);
    Serial.println("======Safety Info======");
    Serial.print("Runtime (s): ");
    Serial.println(seconds);
    Serial.print("Levels: {");
    for (int i=0; i<=(sizeof(level_requirements) / sizeof(level_requirements[0]))-1; i++) {
      Serial.print(level_requirements[i]);
      Serial.print(" ");
    }
    Serial.println("}");
    Serial.println("=================");
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
    digitalWrite(sparkPin, LOW);
    // currentPulse = lastPulse;
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Serial.print("RPM: ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RPM: ");
    lcd.setCursor(5, 0);

    if (currentPulse - lastPulse == 0) {
      // Serial.println(0);
      lcd.print(0);
    } else {
      // Serial.println(60000/(currentPulse-lastPulse));
      lcd.print(60000/(currentPulse-lastPulse));
    }
    lastPulse = currentPulse;

    lcd.setCursor(0, 1);
    lcd.print("Time (s): ");
    lcd.setCursor(10, 1);
    lcd.print(seconds + currentMillis/1000);
  }

  // write runtime
  if (currentMillis - previousMillisWrite >= write_interval) {
    previousMillisWrite = currentMillis;

    if (profile_status) {
      // Serial.println(String(level_requirements[0]) + "\n" + String(level_requirements[1]) + "\n" + String(seconds + currentMillis/1000));
      String data = String(level_requirements[0]) + "\n" + String(level_requirements[1]) + "\n" + String(seconds + currentMillis/1000);
      data.toCharArray(dataBuffer, 100);
      int inputStringLength = strlen(dataBuffer);

      int encodedLength = Base64.encodedLength(inputStringLength);
      char encodedString[encodedLength + 1];
      Base64.encode(encodedString, dataBuffer, inputStringLength);

      engine_profile = SD.open("SAFETY.PRO", FILE_WRITE | O_TRUNC);
      if (engine_profile) {
        engine_profile.println(encodedString);
        engine_profile.close();
      }
      
    }
  }
}

void spark() {
  // Serial.println("Spark");
  digitalWrite(sparkPin, HIGH);
}
