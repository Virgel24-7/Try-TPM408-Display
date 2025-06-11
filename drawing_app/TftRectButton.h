#ifndef TFT_RECT_BUTTON_H  // Include guard to prevent double inclusion
#define TFT_RECT_BUTTON_H

#include <Adafruit_GFX.h>    // Required for Adafruit_ST7789
#include <Adafruit_ST7789.h> // For tft object

class TftRectButton {
  public:
    void init(Adafruit_ST7789 &newTft, int16_t newX0, int16_t newY0, int16_t newWidth, int16_t newHeight, uint16_t btnColor);
    void changeLabel(String text, int16_t textSize, uint16_t textColor);
    bool isPressed(int16_t xloc, int16_t yloc);
    void highlight(int16_t padding, uint16_t color);

  protected:
    int16_t x0, y0, width, height;
    uint16_t color;
    Adafruit_ST7789 *tft;
};

#endif