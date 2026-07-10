#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#define TMP235_PIN A0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SDA_PIN 2
#define SCL_PIN 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println("ADC TMP235");
  Wire1.setSDA(SDA_PIN);
  Wire1.setSCL(SCL_PIN);
  Wire1.begin();
  Wire1.setClock(400000);
  Serial.println("Inicializando TMP235...");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Error con 0x3C, probando 0x3D...");
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("Error OLED en ambas direcciones");
      return;
    }
  }
    Serial.println("OLED inicializada!");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("TMP235 Test");
  display.display();
}

void loop(){
  int raw = analogRead(TMP235_PIN);        // 0 a 4095
  float voltage = raw * 3.3 / 4096.0;      // Voltaje real
  float tempC = (voltage - 0.5) * 100.0;   // Fórmula TMP235
  // OLED
  display.clearDisplay();

  // Título grande arriba
  display.setTextSize(2);
  display.setCursor(28, 0);
  display.println("TMP235");
  // Línea separadora bajo el título
  display.drawFastHLine(0, 18, SCREEN_WIDTH, SSD1306_WHITE);
  // Volver a tamaño normal para los datos
  display.setTextSize(1);

  display.setCursor(0, 22);
  display.print("Raw");
  display.setCursor(70, 22);
  display.print("Voltage");
  display.setCursor(25, 42);
  display.print("Temperature");

  display.setCursor(0, 32);
  display.print("raw=");
  display.print(raw);
  Serial.print("raw=");
  Serial.print(raw);
  
  display.setCursor(70, 32);
  display.print("V=");
  display.print(voltage,2);
  Serial.print("  V=");
  Serial.print(voltage, 3);

  display.setCursor(25, 52);
  display.print("T=");
  display.print(tempC,2);
  Serial.print("  T=");
  Serial.print(tempC, 2);
  Serial.println(" C");

  display.display();

  delay(20);
}