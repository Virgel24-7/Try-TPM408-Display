#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>

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

// --- Custom Colors (16-bit) ---
#define ST77XX_BROWN 0x8B47

// --- Objects ---
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);

//Constants
const int numsize = 4;
const int sizes[numsize] = {6, 12, 18, 24}; //Use even values for more accurate results.
const int numcolors = 8;
const int colors[numcolors] = {ST77XX_RED, ST77XX_YELLOW, ST77XX_GREEN, ST77XX_CYAN, ST77XX_BROWN, ST77XX_ORANGE, ST77XX_BLACK, ST77XX_WHITE};

//Global States
int prevcolidx = 0;
int colidx = 0;
int prevsizeidx = 0;
int sizeidx = 0;

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Serial.println("ST7789 + XPT2046 Touch Example");

  // Initialize SPI once globally
  SPI.begin();

  // Initialize display
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(0);
  tft.invertDisplay(false);
  tft.fillScreen(ST77XX_BLACK);

  createSizing();
  createPallet();
  createClear();

  // Initialize touchscreen
  ts.begin();
  ts.setRotation(0);  // Match display rotation
}

// --- Main Loop ---
void loop() {
  // --- State ---
  static int xwidthcolor = tft.width()/numcolors;
  static int xwidthsize = tft.width()/numsize;

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

    Serial.print("Touch X: "); Serial.print(x);
    Serial.print("  Y: "); Serial.println(y);

    if(y >= 290) {
      clearCanvas();
    }

    else if(y >= 260) {
      for(int i = 0; i < numcolors; i++) {
        if(x < xwidthcolor * (i + 1)) {
          colidx = i;
          if(prevcolidx != colidx) {
            tft.fillCircle(xwidthcolor * colidx + (xwidthcolor / 2), 274, 5, ~(colors[colidx]));
            tft.fillCircle(xwidthcolor * prevcolidx + (xwidthcolor / 2), 274, 5, colors[prevcolidx]);
            prevcolidx = colidx;
          }
          break;
        }
      }
    }

    else if(y < 30) {
      for(int i = 0; i < numsize; i++) {
        if(x < xwidthsize * (i + 1)) {
          sizeidx = i;
          if(prevsizeidx != sizeidx) {
            tft.drawRect(xwidthsize * sizeidx + 5, 5, xwidthsize - 10, 20, ST77XX_BLACK);
            tft.drawRect(xwidthsize * prevsizeidx + 5, 5, xwidthsize - 10, 20, ST77XX_WHITE);
            prevsizeidx = sizeidx;
          }
          break;
        }
      }
    }

    else if(y > 40 && y < 250) {
      int pensize = sizes[sizeidx];
      int starty = constrain(y - (pensize/2), 40, 250);
      int endy = constrain(y + (pensize/2), 40, 250);

      tft.fillRect(x - (pensize/2), starty, pensize, endy - starty, colors[colidx]);
    }
  }
}

void createSizing() {
  int xwidth = tft.width()/numsize;
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);

  for(int i = 0; i < numsize; i++) {
    tft.fillRect(xwidth * i, 0, xwidth, 30, ST77XX_WHITE);
    tft.setCursor(xwidth * i + (xwidth / 2), 12);
    tft.print(String(sizes[i]));
  }

  tft.drawRect(xwidth * sizeidx + 5, 5, xwidth - 10, 20, ST77XX_BLACK);
}

void createPallet() {
  tft.fillRect(0, 259, tft.width(), 1, ST77XX_WHITE);
  int xwidth = tft.width()/numcolors;

  for(int i = 0; i < numcolors; i++) {
    tft.fillRect(xwidth * i, 260, xwidth, 30, colors[i]);
  }

  tft.fillCircle(xwidth * colidx + (xwidth / 2), 274, 5, ~(colors[colidx]));
}

void createClear() {
  tft.fillRect(0, 289, tft.width(), 1, ST77XX_WHITE);
  tft.fillRect(0, 290, tft.width(), 30, ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(90, 297);
  tft.print("CLEAR");
}

void clearCanvas() {
  tft.fillRect(0, 30, 240, 220, ST77XX_BLACK);
}
