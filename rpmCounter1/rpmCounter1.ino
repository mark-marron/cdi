const int analogInputPin = A0;
const int sparkPin = 2;
int count = 0;
bool status = false;

unsigned long previousMillis = 0;
const long interval = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(sparkPin, OUTPUT);
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
    // Serial.print("Count: ");
    // Serial.println(count);
    count = 0;
  }
  // Serial.println(count);
}

void spark() {
  digitalWrite(sparkPin, LOW);
}
