#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "SparkFun_BMI270_Arduino_Library.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SDA_PIN 2
#define SCL_PIN 3

BMI270 imu;
// I2C address selection
uint8_t addr_imu_1 = BMI2_I2C_PRIM_ADDR;  // 0x68
uint8_t addr_imu_2 = BMI2_I2C_SEC_ADDR; 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println("BMI270 Example I2C - Basic Readings I2C with OLED print");
  Wire1.setSDA(SDA_PIN);
  Wire1.setSCL(SCL_PIN);
  Wire1.begin();
  Wire1.setClock(400000);

  bool imu_ok = false;
  Serial.println("Inicializando BMI270...");

  if (imu.beginI2C(addr_imu_1, Wire1) == BMI2_OK) {
    Serial.println("BMI270 detectado en 0x68");
    imu_ok = true;
  } else if (imu.beginI2C(addr_imu_2, Wire1) == BMI2_OK) {
    Serial.println("BMI270 detectado en 0x69");
    imu_ok = true;
  }
  while (!imu_ok) {
    Serial.println("Error: BMI270 no detectado en 0x68 ni 0x69");
    delay(1000);
  }

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
  display.println("BMI270 Test");
  display.display();
}

void loop() {
  imu.getSensorData();

  // Serial (debug completo)
  Serial.print("Acceleration in g's\t");
  Serial.print("X: "); Serial.print(imu.data.accelX, 3); Serial.print("\t");
  Serial.print("Y: "); Serial.print(imu.data.accelY, 3); Serial.print("\t");
  Serial.print("Z: "); Serial.println(imu.data.accelZ, 3);

  Serial.print("Rotation in deg/sec\t");
  Serial.print("X: "); Serial.print(imu.data.gyroX, 3); Serial.print("\t");
  Serial.print("Y: "); Serial.print(imu.data.gyroY, 3); Serial.print("\t");
  Serial.print("Z: "); Serial.println(imu.data.gyroZ, 3);

  // OLED
  display.clearDisplay();

  // Título grande arriba
  display.setTextSize(2);
  display.setCursor(28, 0);
  display.println("BMI270");

  // Línea separadora bajo el título
  display.drawFastHLine(0, 18, SCREEN_WIDTH, SSD1306_WHITE);

  // Volver a tamaño normal para los datos
  display.setTextSize(1);

  // Encabezados de columna
  display.setCursor(0, 22);
  display.print("Accel");
  display.setCursor(70, 22);
  display.print("Gyro");

  // Columna izquierda: aceleración
  display.setCursor(0, 32);
  display.print("X:"); display.println(imu.data.accelX, 2);
  display.setCursor(0, 42);
  display.print("Y:"); display.println(imu.data.accelY, 2);
  display.setCursor(0, 52);
  display.print("Z:"); display.println(imu.data.accelZ, 2);

  // Columna derecha: giroscopio
  display.setCursor(70, 32);
  display.print("X:"); display.println(imu.data.gyroX, 2);
  display.setCursor(70, 42);
  display.print("Y:"); display.println(imu.data.gyroY, 2);
  display.setCursor(70, 52);
  display.print("Z:"); display.println(imu.data.gyroZ, 2);

  display.display();

  delay(20);
}