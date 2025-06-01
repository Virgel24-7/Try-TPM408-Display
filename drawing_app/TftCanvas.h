#ifndef TFT_CANVAS_H
#define TFT_CANVAS_H

#include "TftRectButton.h"  // Parent class
#include <Adafruit_ST7789.h> // For tft object

class TftCanvas : public TftRectButton {
  public:
    void clear();
    void fillColor(uint16_t fillColor);
    void draw(int16_t xloc, int16_t yloc, int penSize, uint16_t penColor);
};

#endif