#include <Arduino.h>
#include "ssd1306.h"
#include "ssd1306_console.h"

#include "nano_engine.h"
#include "sova.h"

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

uint8_t rotation = 0;

static void bitmapDemo()
{
  ssd1306_setColor(RGB_COLOR8(64, 64, 255));
  if (rotation & 1)
    ssd1306_drawMonoBitmap8(0, 0, 64, 64, Owl_64x64);
  else
    ssd1306_drawMonoBitmap8(0, 0, 96, 64, Owl);
  ssd1306_drawBitmap8(0, 0, 8, 8, heartImage8);
  ssd1306_setColor(RGB_COLOR8(255, 64, 64));
  ssd1306_drawMonoBitmap8(0, 16, 8, 8, heartImage);
  delay(3000);
}

/* Sprites are not implemented for color modes.
 * But there is NanoEngine support
 * To make example clear, we use lambda as function pointer. Since lambda can be
 * passed to function only if it doesn't capture, all variables should be global.
 * Refer to C++ documentation.
 */
NanoPoint sprite;
NanoEngine8 engine;
static void spriteDemo()
{
  // We not need to clear screen, engine will do it for us
  engine.begin();
  // Force engine to refresh the screen
  engine.refresh();
  // Set function to draw our sprite
  engine.drawCallback([]() -> bool {
    engine.canvas.clear();
    engine.canvas.setColor(RGB_COLOR8(255, 32, 32));
    engine.canvas.drawBitmap1(sprite.x, sprite.y, 8, 8, heartImage);
    return true;
  });
  sprite.x = 0;
  sprite.y = 0;
  for (int i = 0; i < 250; i++)
  {
    delay(15);
    // Tell the engine to refresh screen at old sprite position
    engine.refresh(sprite.x, sprite.y, sprite.x + 8 - 1, sprite.y + 8 - 1);
    sprite.x++;
    if (sprite.x >= ssd1306_displayWidth())
    {
      sprite.x = 0;
    }
    sprite.y++;
    if (sprite.y >= ssd1306_displayHeight())
    {
      sprite.y = 0;
    }
    // Tell the engine to refresh screen at new sprite position
    engine.refresh(sprite.x, sprite.y, sprite.x + 8 - 1, sprite.y + 8 - 1);
    // Do refresh required parts of screen
    engine.display();
  }
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

static void canvasDemo()
{
  uint8_t buffer[64 * 16 / 8];
  NanoCanvas1_8 canvas(64, 16, buffer);
  ssd1306_setColor(RGB_COLOR8(0, 255, 0));
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen8();
  canvas.clear();
  canvas.fillRect(10, 3, 80, 5);
  canvas.blt((ssd1306_displayWidth() - 64) / 2, 1);
  delay(500);
  canvas.fillRect(50, 1, 60, 15);
  canvas.blt((ssd1306_displayWidth() - 64) / 2, 1);
  delay(1500);
  canvas.printFixed(20, 1, " DEMO ", STYLE_BOLD);
  canvas.blt((ssd1306_displayWidth() - 64) / 2, 1);
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
  Serial.println("loop");
  delay(1000);
  switch (ssd1306_menuSelection(&menu))
  {
  case 0:
    bitmapDemo();
    break;

  case 1:
    spriteDemo();
    break;

  case 2:
    textDemo();
    break;

  case 3:
    canvasDemo();
    break;

  case 4:
    drawLinesDemo();
    break;

  default:
    break;
  }
  // if ((menu.count - 1) == ssd1306_menuSelection(&menu))
  // {
  //   ssd1331_setRotation((++rotation) & 0x03);
  // }
  ssd1306_clearScreen8();
  ssd1306_setColor(RGB_COLOR8(255, 255, 255));
  ssd1306_showMenu8(&menu);
  delay(500);
  ssd1306_menuDown(&menu);
  ssd1306_updateMenu8(&menu);
}