#include "TftCanvas.h"
#include <SPI.h>

void TftCanvas::clear() {
  tft->fillRect(x, y, w, h, color);
}

void TftCanvas::draw(int16_t xloc, int16_t yloc, int penSize, uint16_t penColor) {
  int starty = constrain(yloc - (penSize/2), y, y + h);
  int endy = constrain(yloc + (penSize/2), y, y + h);

  tft->fillRect(xloc - (penSize/2), starty, penSize, endy - starty, penColor);
}