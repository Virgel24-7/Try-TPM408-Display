#include "TftRectButton.h"
#include <SPI.h>

// --- Method Implementations ---
void TftRectButton::init(Adafruit_ST7789 &newTft, int16_t xLocation, int16_t yLocation, int16_t width, int16_t height, uint16_t btnColor) {
  x = xLocation;
  y = yLocation;
  w = width;
  h = height;
  color = btnColor;
  tft = &newTft;
  tft->fillRect(x, y, w, h, color);
}

void TftRectButton::addLabel(String text, int16_t textSize, int16_t topPadding, uint16_t textColor) {
  tft->setTextSize(textSize);
  tft->setTextColor(textColor);

  int16_t textX, textY;
  uint16_t textW, textH;
  tft->getTextBounds(text, 0, 0, &textX, &textY, &textW, &textH);

  int16_t cursorX = x + (w - textW) / 2;
  int16_t cursorY = y + topPadding;
  tft->setCursor(cursorX, cursorY);
  tft->print(text);
}

bool TftRectButton::isPressed(int16_t xloc, int16_t yloc) {
  return (xloc >= x && xloc <= x + w && yloc >= y && yloc <= y + h);
}

void TftRectButton::highlight(int16_t padding, uint16_t color) {
  tft->drawRect(x + padding, y + padding, w - (padding * 2), h - (padding * 2), color);
}