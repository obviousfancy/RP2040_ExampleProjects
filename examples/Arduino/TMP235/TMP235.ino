#define TMP235_PIN 26   // A0 = GPIO26 = ADC0 en el core arduino-pico

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);   // Fuerza los 12 bits nativos (default es 10 bits)
}

void loop() {
  int raw = analogRead(TMP235_PIN);        // 0 a 4095
  float voltage = raw * 3.3 / 4096.0;      // Voltaje real
  float tempC = (voltage - 0.5) * 100.0;   // Fórmula TMP235

  Serial.print("raw=");
  Serial.print(raw);
  Serial.print("  V=");
  Serial.print(voltage, 3);
  Serial.print("  T=");
  Serial.print(tempC, 2);
  Serial.println(" C");

  delay(500);
}