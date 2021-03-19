#include <Arduino.h>
#include "ssd1306.h"
#include "SparkFun_SCD30_Arduino_Library.h"

SCD30 airSensor;

void calibrateSensor()
{
  // measurement interval in seconds
  airSensor.setMeasurementInterval(2);
  // approx. altitude of Dresden, Saxony in meters
  airSensor.setAltitudeCompensation(112);
  // approx. avg. pressure in Dresden, Saxony in mBar
  airSensor.setAmbientPressure(1020);
  // estimated temperature difference between the sensor and its
  // surroundings that would be caused by self-heating during operation
  airSensor.setTemperatureOffset(2);
}

void startSensor()
{
  bool sensorReady = airSensor.begin();
  if (!sensorReady)
  {
    Serial.println("Sensor wiring or setup faulty. Not continuing...");
    while (true)
    {
      ;
    }
  }
}

void waitForSerialConnection()
{
  while (!Serial)
  {
    ;
  }
}

void startSerialConnection()
{
  Serial.begin(9600);
  waitForSerialConnection();
}

void setupI2C()
{
  Wire.begin();
}

void setup()
{
  startSerialConnection();

  setupI2C();
  startSensor();
  calibrateSensor();

  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1331_96x64_spi_init(14, 4, 21); // (gpio14=RST, gpio4=CS, gpio21=D/C)
  // VCCEN needs to be pulled to VCC to turn on the display

  // RGB functions do not work in default SSD1306 compatible mode
  ssd1306_setMode(LCD_MODE_NORMAL);
  ssd1306_clearScreen8();
}

int co2 = 0;
float temperature = 0;

void loop()
{
  if (airSensor.dataAvailable())
  {
    co2 = airSensor.getCO2();
    temperature = airSensor.getTemperature();
  }

  delay(1000);
  String text = String(co2) + " " + String(temperature);
  ssd1306_clearScreen8();
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  ssd1306_setColor(RGB_COLOR8(102, 255, 51));
  char text2[12];
  text.toCharArray(text2, 12);
  ssd1306_printFixed8(4, 48, text2, STYLE_BOLD);

  ssd1331_drawLine(0, 0, 15, 15, RGB_COLOR8(0, 0, 255));
}