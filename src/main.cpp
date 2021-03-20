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
  delay(1500); // another short while to let the serial monitor start in time
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

int history[96] = {};

void setup()
{
  startSerialConnection();

  setupI2C();
  startSensor();
  calibrateSensor();

  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);

  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1331_96x64_spi_init(14, 4, 21); // (gpio14=RST, gpio4=CS, gpio21=D/C)
  // VCCEN needs to be pulled to VCC to turn on the display

  // RGB functions do not work in default SSD1306 compatible mode
  ssd1306_setMode(LCD_MODE_NORMAL);
  ssd1306_clearScreen8();

  for (int i = 0; i < 96; i++)
  {
    history[i] = 400;
  }
}

int co2 = 0;
float temperature = 0;

void shiftHistory()
{
  for (int i = 0; i < 95; i++)
  {
    history[i] = history[i + 1];
  }
}

void loop()
{
  if (airSensor.dataAvailable())
  {
    co2 = airSensor.getCO2();
    history[95] = co2;
    temperature = airSensor.getTemperature();

    ssd1306_clearScreen8();
    ssd1306_setFixedFont(ssd1306xled_font8x16);

    String co2asString = String(co2);
    String temperatureString = String(temperature);

    char co2Text[5];
    co2asString.toCharArray(co2Text, 5);
    char temperatureText[6];
    temperatureString.toCharArray(temperatureText, 6);
    int textX = 0;
    if (co2 >= 10000)
    {
      textX = 4;
    }
    else if (co2 >= 1000 && co2 < 10000)
    {
      textX = 12;
    }
    else
    {
      textX = 20;
    }

    ssd1306_setColor(RGB_COLOR8(0, 255, 0));
    if (co2 >= 900)
    {
      ssd1306_setColor(RGB_COLOR8(255, 255, 0));
    }
    if (co2 >= 1500)
    {
      ssd1306_setColor(RGB_COLOR8(255, 0, 0));
    }

    ssd1306_printFixed8(textX, 48, co2Text, STYLE_BOLD);

    ssd1306_setColor(RGB_COLOR8(255, 255, 255));
    ssd1306_printFixed8(50, 48, temperatureText, STYLE_BOLD);

    for (int i = 0; i < 96; i++)
    {
      int y = 47 - min(47, ((history[i] - 400) / 20));
      ssd1331_drawLine(i, 47, i, y, RGB_COLOR8(0, 255, 0));
      if (history[i] >= 900)
      {
        // ssd1331_drawLine(i, 22, i, y, RGB_COLOR8(0, 255, 255)); // seems to hit the right pixel
        ssd1331_drawLine(i, 47, i, y, RGB_COLOR8(0, 255, 255));
      }

      if (history[i] >= 1500)
      {
        ssd1331_drawLine(i, 47, i, y, RGB_COLOR8(0, 0, 255));
      }
    }
    shiftHistory();
  }
}