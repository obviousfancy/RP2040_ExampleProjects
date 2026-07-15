#include <SPI.h>
#include <Wire.h>
#include "SparkFun_BMI270_Arduino_Library.h"
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Bus I2C (OLED) - bloque I2C0
#define SDA_PIN 0
#define SCL_PIN 1

// Bus SPI (BMI270) - bloque SPI0
#define PIN_SCLK  2  // scl
#define PIN_MISO  4  // sdo
#define PIN_MOSI  3  // sda
#define PIN_CS    5  // cs

BMI270 imu;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Parametro de reloj SPI. Se deja en 100 kHz por margen de estabilidad en
// esta practica; una vez verificado el enlace, puede incrementarse (el
// BMI270 admite hasta 10 MHz en modo SPI) para reducir el tiempo de
// transaccion por lectura.
uint32_t clockFrequency = 100000;

void setup()
{
    Serial.begin(115200);
    Serial.println("BMI270 Example SPI - Basic Readings SPI with OLED print");

    // Bus I2C para el OLED
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
    SPI.begin();

    // Check if sensor is connected and initialize
    // Clock frequency is optional (defaults to 100kHz)
    while (imu.beginSPI(PIN_CS, clockFrequency) != BMI2_OK)
    {
        Serial.println("Error: BMI270 not connected, check wiring and CS pin!");
        delay(1000);
    }

    Serial.println("BMI270 connected!");

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Error con 0x3C, probando 0x3D...");
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
            Serial.println("Error OLED en ambas direcciones");
            return;
        }
    }

    Serial.println("OLED inicializada!");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("BMI270 Test");
    display.display();
}

void loop()
{
    // Get measurements from the sensor. This must be called before accessing
    // the sensor data, otherwise it will never update
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

    // Titulo grande arriba
    display.setTextSize(2);
    display.setCursor(28, 0);
    display.println("BMI270");

    // Linea separadora bajo el titulo
    display.drawFastHLine(0, 18, SCREEN_WIDTH, SSD1306_WHITE);

    // Volver a tamano normal para los datos
    display.setTextSize(1);

    // Encabezados de columna
    display.setCursor(0, 22);
    display.print("Accel");
    display.setCursor(70, 22);
    display.print("Gyro");

    // Columna izquierda: aceleracion
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
