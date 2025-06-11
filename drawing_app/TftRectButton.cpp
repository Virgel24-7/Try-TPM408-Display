#include "TftRectButton.h"
#include <SPI.h>

// --- Method Implementations ---
void TftRectButton::init(Adafruit_ST7789 &newTft, int16_t newX0, int16_t newY0, int16_t newWidth, int16_t newHeight, uint16_t btnColor) {
  x0 = newX0;
  y0 = newY0;
  width = newWidth;
  height = newHeight;
  color = btnColor;
  tft = &newTft;
  tft->fillRect(x0, y0, width, height, color);
}

void TftRectButton::changeLabel(String text, int16_t textSize, uint16_t textColor) {
  tft->fillRect(x0, y0, width, height, color);
  tft->setTextSize(textSize);
  tft->setTextColor(textColor);

  int16_t textX, textY;
  uint16_t textW, textH;
  tft->getTextBounds(text, 0, 0, &textX, &textY, &textW, &textH);

  int16_t cursorX = x0 + (width - textW) / 2;
  int16_t cursorY = y0 + (height - textH) / 2;
  tft->setCursor(cursorX, cursorY);
  tft->print(text);
}

bool TftRectButton::isPressed(int16_t xloc, int16_t yloc) {
  return (xloc >= x0 && xloc <= x0 + width && yloc >= y0 && yloc <= y0 + height);
}

void TftRectButton::highlight(int16_t padding, uint16_t color) {
  tft->drawRect(x0 + padding, y0 + padding, width - (padding * 2), height - (padding * 2), color);
}