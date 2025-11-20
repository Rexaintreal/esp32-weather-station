#define PIR_PIN 18

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  Serial.println("Wait 30–60s");
}

void loop() {
  int motion = digitalRead(PIR_PIN);

  if (motion) {
    Serial.println("MOTION!");
  } else {
    Serial.println("No motion");
  }

  delay(300);
}
