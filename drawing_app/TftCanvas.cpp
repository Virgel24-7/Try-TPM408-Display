#include "TftCanvas.h"
#include <SPI.h>

void TftCanvas::accessInternet() {
  accessnet = true;
}

bool TftCanvas::createBuffer() {
  if(width == 0 || height == 0) {
    return false;
  }
  size_t bufferSize = width * height;
  canvasBuffer = (char*)malloc(bufferSize + 1);
  if(!canvasBuffer) return false;

  tft->drawRect(x0-2,y0-2,width+4,height+4,ST77XX_WHITE);
  memset(canvasBuffer, (char)(33), bufferSize);
  canvasBuffer[bufferSize + 1] = '\0';
  return true;
}

void TftCanvas::clear() {
  tft->fillRect(x0, y0, width, height, color);
  if(accessnet) memset(canvasBuffer, (char)(33), width * height);
}

void TftCanvas::fillColor(uint16_t fillColor, int coloridx) {
  tft->fillRect(x0, y0, width, height, fillColor);
  if(accessnet) memset(canvasBuffer, (char)(coloridx + 33), width * height);
}

void TftCanvas::draw(int16_t xloc, int16_t yloc, int penSize, uint16_t penColor, int coloridx) {
  int startx = constrain(xloc - (penSize/2), x0, x0 + width);
  int endx = constrain(xloc + (penSize/2), x0, x0 + width);
  int starty = constrain(yloc - (penSize/2), y0, y0 + height);
  int endy = constrain(yloc + (penSize/2), y0, y0 + height);

  tft->fillRect(startx, starty, endx - startx, endy - starty, penColor);

  if(!accessnet) return;

  int penx0 = startx - x0;
  int peny0 = starty - y0;
  int penWidth = endx - startx;
  int penHeight = endy - starty;

  for (int i = peny0; i < peny0 + penHeight; i++) {
    for (int j = penx0; j < penx0 + penWidth; j++) {
      setPixel(j, i, coloridx);
    }
  }
}

void TftCanvas::setPixel(int xloc, int yloc, uint8_t coloridx) {
  if (!canvasBuffer) return;
  if (xloc < 0 || xloc >= width || yloc < 0 || yloc >= height) return;

  char ch = (char)(coloridx + 33);
  if (ch > 126) ch = 126;
  canvasBuffer[yloc * width + xloc] = ch;
}

char* const TftCanvas::getCanvasBuffer() const {
  return canvasBuffer;
}

int16_t TftCanvas::getCanvasWidth() {
  return width;
}

int16_t TftCanvas::getCanvasHeight() {
  return height;
}