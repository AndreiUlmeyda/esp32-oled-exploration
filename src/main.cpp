#include <Arduino.h>
#include "ssd1306.h"
#include "ssd1306_console.h"
#include "SparkFun_SCD30_Arduino_Library.h"

#include "nano_engine.h"
#include "sova.h"

SCD30 airSensor;

/*
 * Heart image below is defined directly in flash memory.
 * This reduces SRAM consumption.
 * The image is defined from bottom to top (bits), from left to
 * right (bytes).
 */
const PROGMEM uint8_t heartImage[8] =
    {
        0B00001110,
        0B00011111,
        0B00111111,
        0B01111110,
        0B01111110,
        0B00111101,
        0B00011001,
        0B00001110};

const PROGMEM uint8_t heartImage8[8 * 8] =
    {
        0x00,
        0xE0,
        0xE0,
        0x00,
        0x00,
        0xE5,
        0xE5,
        0x00,
        0xE0,
        0xC0,
        0xE0,
        0xE0,
        0xE0,
        0xEC,
        0xEC,
        0xE5,
        0xC0,
        0xE0,
        0xE0,
        0xE0,
        0xE0,
        0xE5,
        0xEC,
        0xE5,
        0x80,
        0xC0,
        0xE0,
        0xE0,
        0xE0,
        0xE0,
        0xE5,
        0xE0,
        0x00,
        0x80,
        0xC0,
        0xE0,
        0xE0,
        0xE0,
        0xE0,
        0x00,
        0x00,
        0x00,
        0x80,
        0xE0,
        0xE0,
        0xE0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x80,
        0xE0,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};

SAppMenu menu;

const char *menuItems[] =
    {
        "draw lines",
        "fonts",
};

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
  delay(1000); // so that the serial monitor terminal has time to catch up
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
  ssd1331_96x64_spi_init(14, 4, 21); // (gpio22=RST, gpio5=CE for VSPI, gpio21=D/C)
  // VCCEN needs to be pulled to VCC to turn on the display

  // RGB functions do not work in default SSD1306 compatible mode
  ssd1306_setMode(LCD_MODE_NORMAL);
  ssd1306_clearScreen8();
}

NanoEngine8 engine;

static void textDemo()
{
  ssd1306_clearScreen8();
  ssd1306_setFixedFont(digital_font5x7);
  ssd1306_setColor(RGB_COLOR8(0, 64, 255));
  ssd1306_printFixed8(0, 0, "0123456789", STYLE_NORMAL);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_setColor(RGB_COLOR8(255, 255, 0));
  ssd1306_printFixed8(0, 8, "Normal text", STYLE_NORMAL);
  ssd1306_setColor(RGB_COLOR8(0, 255, 0));
  ssd1306_printFixed8(0, 16, "Bold text?", STYLE_BOLD);
  ssd1306_setColor(RGB_COLOR8(0, 255, 255));
  ssd1306_printFixed8(0, 24, "Italic text?", STYLE_ITALIC);
  ssd1306_negativeMode();
  ssd1306_setColor(RGB_COLOR8(255, 255, 255));
  ssd1306_printFixed8(0, 32, "Inverted bold?", STYLE_BOLD);
  ssd1306_positiveMode();
  delay(3000);
}

static void drawLinesDemo()
{
  /* SSD1331 controller has hardware acceleration, thus                *
     * use hw ssd1331_drawLine() instead of software ssd1331_drawLine8() */
  ssd1306_clearScreen();
  for (uint8_t y = 0; y < ssd1306_displayHeight(); y += 8)
  {
    ssd1331_drawLine(0, 0, ssd1306_displayWidth() - 1, y, RGB_COLOR8(0, 255, 0));
  }
  for (uint8_t x = ssd1306_displayWidth() - 1; x > 7; x -= 8)
  {
    ssd1331_drawLine(0, 0, x, ssd1306_displayHeight() - 1, RGB_COLOR8(0, 0, 255));
  }
  delay(3000);
}

void loop()
{
  int co2 = airSensor.getCO2();
  float temperature = airSensor.getTemperature();

  delay(1000);
  String text = String(co2);
  ssd1306_clearScreen8();
  ssd1306_setFixedFont(ssd1306xled_font8x16);
  ssd1306_setColor(RGB_COLOR8(153, 0, 204));
  char text2[5];
  text.toCharArray(text2, 5);
  ssd1306_printFixed8(25, 25, text2, STYLE_BOLD);
}