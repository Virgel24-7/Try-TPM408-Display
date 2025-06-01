#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>
#include "TftRectButton.h"
#include "TftCanvas.h"

// --- Colors (16-bit) ---
#define BACKGROUND ST77XX_BLACK //Background of canvas
#define ST77XX_BROWN 0x8B47

// --- Constants (change to accomodate more sizes and colors) ---
const int numsizes = 5;
const int sizes[numsizes] = {2, 6, 12, 18, 24}; //Use even values for more accurate result.
const int numcolors = 9;
const int colors[numcolors] = {ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE, ST77XX_YELLOW, ST77XX_CYAN, ST77XX_ORANGE, ST77XX_BROWN, ST77XX_BLACK, ST77XX_WHITE};

// --- Display Pins ---
#define TFT_CS   D2   // GPIO4
#define TFT_DC   D1   // GPIO5
#define TFT_RST  D0   // GPIO16

// --- Touchscreen Pins ---
#define TS_CS    D8   // GPIO15
#define TS_IRQ   -1   // Not used in this example

// --- Screen Dimensions ---
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// --- SPI Settings for Touch ---
#define TS_SPI_SPEED 2000000  // 2 MHz (safe for XPT2046)
SPISettings tsSPISettings(TS_SPI_SPEED, MSBFIRST, SPI_MODE0);

// --- Objects ---
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);

// --- Global States ---
int prevcolidx = 0;
int colidx = 0;
int prevsizeidx = 0;
int sizeidx = 0;

// --- Custom objects ---
TftRectButton clearbtn;
TftRectButton sizefield;
TftRectButton sizebtns[numsizes];
TftRectButton colorfield;
TftRectButton colorbtns[numcolors];
TftCanvas canvas;

// --- Setup ---
void setup() {
  // Initialize SPI once globally
  SPI.begin();

  // Initialize display
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(0);
  tft.invertDisplay(false);
  tft.fillScreen(ST77XX_BLACK);

  //Create the UI
  createSizes(0, 0, 240, 30);
  createColors(0, 250, 240, 30);
  clearbtn.init(tft, 0, 290, tft.width(), 30, ST77XX_BLUE);
  clearbtn.addLabel("CLEAR", 2, 5, ST77XX_WHITE);
  canvas.init(tft, 0, 40, 240, 200, BACKGROUND);

  // Initialize touchscreen
  ts.begin();
  ts.setRotation(0);  // Match display rotation
}

// --- Main Loop ---
void loop() {
  // --- State ---
  static int xwidthcolor = tft.width()/numcolors;
  static int xwidthsize = tft.width()/numsizes;

  // Check touch
  SPI.beginTransaction(tsSPISettings);
  bool touched = ts.touched();
  TS_Point p;
  if (touched) {
    p = ts.getPoint();
  }
  SPI.endTransaction();

  if (touched) {
    // Map raw to screen coordinates
    // Adjust raw range for your specific touch panel
    // Invert axes: low -> high becomes high -> low
    int16_t x = map(p.x, 3800, 200, 0, TFT_WIDTH);   // Inverted X
    int16_t y = map(p.y, 3800, 200, 0, TFT_HEIGHT);  // Inverted Y

    x = constrain(x, 0, TFT_WIDTH - 1);
    y = constrain(y, 0, TFT_HEIGHT - 1);

    if(clearbtn.isPressed(x, y)) {
      canvas.clear();
    }

    else if(colorfield.isPressed(x, y)) {
      for(int i = 0; i < numcolors; i++) {
        if(colorbtns[i].isPressed(x, y)) {
          colidx = i;
          if(prevcolidx != colidx) {
            colorbtns[colidx].highlight(5, ~(colors[colidx]));
            colorbtns[prevcolidx].highlight(5, colors[prevcolidx]);
            prevcolidx = colidx;
          }

          break;
        }
      }
    }

    else if(sizefield.isPressed(x, y)) {
      for(int i = 0; i < numsizes; i++) {
        if(sizebtns[i].isPressed(x, y)) {
          sizeidx = i;
          if(prevsizeidx != sizeidx) {
            sizebtns[sizeidx].highlight(5, ST77XX_BLACK);
            sizebtns[prevsizeidx].highlight(5, ST77XX_WHITE);
            prevsizeidx = sizeidx;
          }
          break;
        }
      }
    }

    else if(canvas.isPressed(x, y)) {
      canvas.draw(x, y, sizes[sizeidx], colors[colidx]);
    }
  }
}

void createSizes(int x, int y, int w, int h) {
  int xwidth = w/numsizes;
  sizefield.init(tft, x, y, w, h, ST77XX_WHITE);

  for(int i = 0; i < numsizes; i++) {
    sizebtns[i].init(tft, xwidth * i, y, xwidth, h, ST77XX_WHITE);
    sizebtns[i].addLabel(String(sizes[i]), 1, 12, ST77XX_BLACK);
  }

  sizebtns[sizeidx].highlight(5, ST77XX_BLACK);
}

void createColors(int x, int y, int w, int h) {
  int xwidth = w/numcolors;
  colorfield.init(tft, x, y, w, h, ST77XX_BLACK);

  for(int i = 0; i < numcolors; i++) {
    colorbtns[i].init(tft, xwidth * i, y, xwidth, h, colors[i]);
  }

  colorbtns[colidx].highlight(5, ~colors[colidx]);
}