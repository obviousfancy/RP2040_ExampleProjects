

const int TOUCH_PIN = 7;              // GP6 — pin físico 9 en el header del Pico
volatile unsigned long pulseCount = 0;

void onPulse() {
  pulseCount++;
  Serial.print("¡Pulso detectado! Total: ");
  Serial.println(pulseCount);
}

void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(TOUCH_PIN), onPulse, RISING);
  Serial.println("Esperando pulsos del sensor capacitivo en GP6...");
}

void loop() {
  delay(1000);
}