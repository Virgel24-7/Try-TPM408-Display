#ifndef TFT_CANVAS_H
#define TFT_CANVAS_H

#include "TftRectButton.h"  // Parent class
#include <Adafruit_ST7789.h> // For tft object

class TftCanvas : public TftRectButton {
  public:
    void accessInternet();
    bool createBuffer();
    void clear();
    void fillColor(uint16_t fillColor, int coloridx);
    void draw(int16_t xloc, int16_t yloc, int penSize, uint16_t penColor, int coloridx);
    void setPixel(int xloc, int yloc, uint8_t coloridx);
    char* const getCanvasBuffer() const;
    int16_t getCanvasWidth();
    int16_t getCanvasHeight();

  private:
    char* canvasBuffer = nullptr;
    bool accessnet = false;
    bool bufferMade = false;
};

#endif