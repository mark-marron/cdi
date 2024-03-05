void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(3, HIGH);
}

void loop() {
  digitalWrite(10, HIGH);
  delay(10);
  digitalWrite(10, LOW);
  delay(10);
}
