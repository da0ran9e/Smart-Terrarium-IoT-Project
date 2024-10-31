#define ACT_PIN 5

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ACT_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ACT_PIN, HIGH);
  Serial.println("ON");
  delay(2000);
  digitalWrite(ACT_PIN, LOW);
  Serial.println("OFF");
  delay(5000);
}
