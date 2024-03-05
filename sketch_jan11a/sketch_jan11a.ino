const int pulsePin = 2;  // Digital pin connected to the dynamo output
volatile int pulseCount = 0;  // Counter variable, marked as volatile for use in interrupts
unsigned long lastDebounceTime = 0;  // Last time the input pin was toggled
unsigned long debounceDelay = 50;  // Delay time in milliseconds to debounce the signal

void setup() {
  pinMode(pulsePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsePin), countPulse, RISING);
  Serial.begin(9600);
}

void loop() {
  // Your main code here (if needed)
  // You can use the pulseCount variable for whatever purpose you like
  // For example, print the count to the serial monitor
  Serial.println(pulseCount);
  delay(1000);  // Adjust delay as needed
}

void countPulse() {
  // This function will be called when a rising edge is detected on the pulsePin
  unsigned long currentTime = millis();

  // Check if enough time has passed since the last toggle to debounce
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    pulseCount++;
    lastDebounceTime = currentTime;
  }
}

