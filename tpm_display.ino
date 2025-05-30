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

// --- Objects ---
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);

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

  // Initialize touchscreen
  ts.begin();
  ts.setRotation(0);  // Match display rotation
}

// --- Main Loop ---
void loop() {
  // --- State ---
  static TS_Point prev;
  static bool hasPrev = false;

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

    // Draw red circle
    if(hasPrev)
    {
      tft.fillCircle(prev.x, prev.y, 20, ST77XX_BLACK);
    }
    tft.fillCircle(x, y, 20, ST77XX_RED);

    //Save state
    prev.x = x;
    prev.y = y;
    hasPrev = true;
  }

  delay(100);  // Small delay to avoid spamming
}
