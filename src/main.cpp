#include <Arduino.h>
#include "ssd1306.h"
#include "ssd1306_console.h"

SAppMenu menu;

const char *menuItems[] =
    {
        "draw bitmap",
        "sprites",
        "fonts",
        "nano canvas",
        "draw lines",
};

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

void setup()
{
  startSerialConnection();
  Serial.println("setup");

  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1331_96x64_spi_init(14, 4, 21); // (gpio22=RST, gpio5=CE for VSPI, gpio21=D/C)
  // VCCEN needs to be pulled to VCC to turn on the display

  // RGB functions do not work in default SSD1306 compatible mode
  ssd1306_setMode(LCD_MODE_NORMAL);
  ssd1306_clearScreen8();
  ssd1306_createMenu(&menu, menuItems, sizeof(menuItems) / sizeof(char *));
  ssd1306_showMenu8(&menu);
}

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

void loop()
{
  Serial.println("loop");
  textDemo();
  delay(1500);
}