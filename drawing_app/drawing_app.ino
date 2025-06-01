#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>
#include "TftRectButton.h"
#include "TftCanvas.h"
#include "Colors.h"   //Modify to change color palette

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// --- Colors (16-bit) ---
#define COLORPERPAGE 7  //Better if this + 1 is a factor of tft.witdth()
#define BACKGROUND 0x0000 //Background of canvas

// --- Constants (change to accomodate more sizes) ---
const int sizes[] = {2, 6, 12, 18, 24, 50}; //Use even values for more accurate result.
const int numsizes = ARRAY_SIZE(sizes);

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
int currcolpage = 1;
int numpages = (NUM_COLORS + COLORPERPAGE - 1) / COLORPERPAGE;
uint16_t currColors[COLORPERPAGE];
int prevcolidx = 0;
int currcolidx = 0;
int prevsizeidx = 0;
int sizeidx = 0;

// --- Custom objects ---
TftRectButton clearbtn;
TftRectButton fillbtn;
TftRectButton sizefield;
TftRectButton sizebtns[numsizes];
TftRectButton colorflip;
TftRectButton colorfield;
TftRectButton colorbtns[NUM_COLORS];
TftCanvas canvas;

// --- Setup ---
void setup() {
  Serial.begin(115200);

  // Initialize SPI once globally
  SPI.begin();

  // Initialize display
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(0);
  tft.invertDisplay(false);
  tft.fillScreen(ST77XX_BLACK);

  //Create the UI
  createSizes(0, 0, 240, 30);
  tft.fillRect(0, 249, 240, 1, ST77XX_WHITE);
  createColors(0, 250, 240, 30);
  tft.fillRect(0, 280, 240, 1, ST77XX_WHITE);
  fillbtn.init(tft, 0, 290, tft.width() / 2, 30, ST77XX_ORANGE);
  fillbtn.addLabel("FILL SCRN", 1, ST77XX_WHITE);
  clearbtn.init(tft, tft.width() / 2, 290, tft.width() / 2, 30, ST77XX_BLUE);
  clearbtn.addLabel("CLEAR", 1, ST77XX_WHITE);
  canvas.init(tft, 0, 40, 240, 200, BACKGROUND);

  // Initialize touchscreen
  ts.begin();
  ts.setRotation(0);  // Match display rotation
}

// --- Main Loop ---
void loop() {
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
    else if(fillbtn.isPressed(x, y)) {
      canvas.fillColor(currColors[currcolidx]);
    }
    else if(colorflip.isPressed(x, y)) {
      currcolpage++;
      if(currcolpage > numpages) currcolpage = 1;
      createColors(0, 250, 240, 30);
    }
    else if(colorfield.isPressed(x, y)) {
      for(int i = 0; i < NUM_COLORS; i++) {
        if(colorbtns[i].isPressed(x, y)) {
          currcolidx = i;
          if(prevcolidx != currcolidx) {
            colorbtns[currcolidx].highlight(5, ~(currColors[currcolidx]));
            colorbtns[prevcolidx].highlight(5, currColors[prevcolidx]);
            prevcolidx = currcolidx;
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
      canvas.draw(x, y, sizes[sizeidx], currColors[currcolidx]);
    }
  }
}

void createSizes(int x, int y, int w, int h) {
  int xwidth = w/numsizes;
  sizefield.init(tft, x, y, w, h, ST77XX_WHITE);

  for(int i = 0; i < numsizes; i++) {
    sizebtns[i].init(tft, xwidth * i, y, xwidth, h, ST77XX_WHITE);
    sizebtns[i].addLabel(String(sizes[i]), 1, ST77XX_BLACK);
  }

  sizebtns[sizeidx].highlight(5, ST77XX_BLACK);
}

void createColors(int x, int y, int w, int h) {
  int xwidth = w/(COLORPERPAGE + 1);
  colorflip.init(tft, x, y, xwidth, h, ST77XX_WHITE);
  colorflip.addLabel(String(currcolpage), 2, ST77XX_BLACK);
  colorflip.highlight(0, ST77XX_BLACK);
  colorfield.init(tft, xwidth, y, w - xwidth, h, ST77XX_BLACK);

  //Get current page colors
  for(int i = 0; i < COLORPERPAGE; i++) {
    int j = i + ((currcolpage - 1) * (COLORPERPAGE));
    currColors[i] = j < NUM_COLORS? colors[j]: ST77XX_BLACK;
    colorbtns[i].init(tft, xwidth * (i + 1), y, xwidth, h, currColors[i]);
  }

  colorbtns[currcolidx].highlight(5, ~currColors[currcolidx]);
}