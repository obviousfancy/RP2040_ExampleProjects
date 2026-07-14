#include <SPI.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SDA_PIN 0
#define SCL_PIN 1

#define PIN_SCLK  2  // scl
#define PIN_MISO  4  // sdo
#define PIN_MOSI  3  // sda
#define PIN_CS    5  // cs

// Create a new sensor object
BMI270 imu;

uint8_t addr_imu_1 = BMI2_I2C_PRIM_ADDR;  // 0x68
uint8_t addr_imu_2 = BMI2_I2C_SEC_ADDR; 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SPI parameters
uint32_t clockFrequency = 100000;

void setup()
{
    // Start serial
    Serial.begin(115200);
    Serial.println("BMI270 Example I2C - Basic Readings I2C with OLED print");
    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();
    Wire.setClock(400000);
    // En el nucleo Arduino-Pico, el mapeo de pines del periferico SPI0 se
    // define ANTES de SPI.begin(). No es necesario remapear CS: la libreria
    // de SparkFun lo maneja directo por GPIO (pinMode/digitalWrite), sin
    // pasar por la clase SPI.
    SPI.setSCK(PIN_SCLK);
    SPI.setTX(PIN_MOSI);
    SPI.setRX(PIN_MISO);

    // Initialize the SPI library
    SPI.begin();

    // Check if sensor is connected and initialize
    // Clock frequency is optional (defaults to 100kHz)
    while (imu.beginSPI(PIN_CS, clockFrequency) != BMI2_OK)
    {
        // Not connected, inform user
        Serial.println("Error: BMI270 not connected, check wiring and CS pin!");
        // Wait a bit to see if connection is established
        delay(1000);
    }

    Serial.println("BMI270 connected!");
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

void loop()
{
    // Get measurements from the sensor. This must be called before accessing
    // the sensor data, otherwise it will never update
    imu.getSensorData();

    // Print acceleration data
    Serial.print("Acceleration in g's");
    Serial.print("\t");
    Serial.print("X: ");
    Serial.print(imu.data.accelX, 3);
    Serial.print("\t");
    Serial.print("Y: ");
    Serial.print(imu.data.accelY, 3);
    Serial.print("\t");
    Serial.print("Z: ");
    Serial.print(imu.data.accelZ, 3);
    Serial.print("\t");

    // Print rotation data
    Serial.print("Rotation in deg/sec");
    Serial.print("\t");
    Serial.print("X: ");
    Serial.print(imu.data.gyroX, 3);
    Serial.print("\t");
    Serial.print("Y: ");
    Serial.print(imu.data.gyroY, 3);
    Serial.print("\t");
    Serial.print("Z: ");
    Serial.println(imu.data.gyroZ, 3);

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