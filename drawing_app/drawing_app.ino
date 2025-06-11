#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>
#include "TftRectButton.h"
#include "TftCanvas.h"
#include "Colors.h"   //Modify to change color palette
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <WiFiClientSecure.h>

#define ENABLE_USER_AUTH
#define ENABLE_DATABASE
#include <FirebaseClient.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define API_KEY ""
#define DATABASE_URL ""
#define USER_EMAIL ""
#define USER_PASSWORD ""

WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// --- Colors and Canvas (16-bit) ---
#define COLORPERPAGE 7  //Better if this + 1 is a factor of tft.witdth()
#define BACKGROUND 0x0000 //Background of canvas
#define CANVAS_WIDTH 240    //Should be less than or equal to 240
#define CANVAS_HEIGHT 200     //Should be less than or equal to 200
#define NETCANVAS_WIDTH 110    //With Internet -- Should be less than or equal to 240
#define NETCANVAS_HEIGHT 90     //With Internet -- Should be less than or equal to 200

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
bool isInternet = false;
int currcolpage = 1;
int numpages = (NUM_COLORS + COLORPERPAGE - 1) / COLORPERPAGE;
uint16_t currColors[COLORPERPAGE];
int prevcolidx = 1;
int currcolidx = 1;
int prevsizeidx = 0;
int sizeidx = 0;
long savetime = 0;
const long msbeforecancel = 120000;   //2 minutes

// --- Custom objects ---
TftRectButton clearbtn;
TftRectButton fillbtn;
TftRectButton savebtn;
TftRectButton sizefield;
TftRectButton sizebtns[numsizes];
TftRectButton colorflip;
TftRectButton colorfield;
TftRectButton colorbtns[NUM_COLORS];
TftCanvas canvas;

bool isBuffer = false;

// --- Setup ---
void setup() {
  //serial.begin(115200);

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

  //Show internet Connectivity option
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.print("SAVE TO INTERNET?");
  TftRectButton Yes; 
  Yes.init(tft, 0, 50, tft.width(), 50, ST77XX_ORANGE);
  Yes.changeLabel("YES(" + String(NETCANVAS_WIDTH) + "x" + String(NETCANVAS_HEIGHT) + ")", 3, ST77XX_WHITE);
  TftRectButton No;
  No.init(tft, 0, 110, tft.width(), 50, ST77XX_BLUE);
  No.changeLabel("NO(" + String(CANVAS_WIDTH) + "x" + String(CANVAS_HEIGHT) + ")", 3, ST77XX_WHITE);

  int16_t x = 0;
  int16_t y = 0;
  while(!Yes.isPressed(x, y) && !No.isPressed(x, y)) {
    bool touched = isTouched(x, y);
    if(Yes.isPressed(x,y))  isInternet = true;

    delay(1);
  }
  tft.fillScreen(ST77XX_BLACK);
  if(isInternet) {
    waitConnection();
    tft.fillScreen(ST77XX_BLACK);
  }

  //Create the size and color selection
  createSizes(0, 0, 240, 30);
  tft.fillRect(0, 249, 240, 1, ST77XX_WHITE);
  createColors(0, 250, 240, 30);
  tft.fillRect(0, 280, 240, 1, ST77XX_WHITE);

  //Create buttons
  int tempw = isInternet? tft.width() / 3: tft.width()/2;

  fillbtn.init(tft, 0, 290, tempw, 30, ST77XX_ORANGE);
  fillbtn.changeLabel("FILL SCRN", 1, ST77XX_WHITE);
  clearbtn.init(tft, tempw, 290, tempw, 30, ST77XX_BLUE);
  clearbtn.changeLabel("CLEAR", 1, ST77XX_WHITE);
 
  if(isInternet) {
    //Create canvas
    canvas.init(tft, (tft.width() - NETCANVAS_WIDTH)/2, 40 + ((200 - NETCANVAS_HEIGHT)/2), NETCANVAS_WIDTH, NETCANVAS_HEIGHT, BACKGROUND);

    savebtn.init(tft, tempw * 2, 290, tempw, 30, ST77XX_ORANGE);
    savebtn.changeLabel("SAVE", 1, ST77XX_WHITE);

    //Let canvas know
    canvas.accessInternet();
    isBuffer = canvas.createBuffer();
    if(!isBuffer) {
      //serial.println("Buffer not created.");
    }
    else {
      //serial.println("Buffer created");
    }
  }
  else {
    canvas.init(tft, (tft.width() - CANVAS_WIDTH)/2, (40 + (200 - CANVAS_HEIGHT)/2), CANVAS_WIDTH, CANVAS_HEIGHT, BACKGROUND);
  }
}

// --- Main Loop ---
void loop() {
  //if(isInternet) //serial.println(WiFi.RSSI());

  // Check touch
  int16_t x;
  int16_t y;
  bool touched = isTouched(x, y);

  if (touched) {
    if(clearbtn.isPressed(x, y)) {
      canvas.clear();
    }
    else if(fillbtn.isPressed(x, y)) {
      int idx = currcolidx + ((currcolpage - 1) * (COLORPERPAGE));
      if(idx >= NUM_COLORS) idx = 0;
      canvas.fillColor(currColors[currcolidx], idx);
    }
    else if(savebtn.isPressed(x, y) && isBuffer) {
      //serial.println("Saving...");
      savebtn.changeLabel("SAVING", 1, ST77XX_WHITE);
      sendToFirebase(canvas.getCanvasBuffer());
      //serial.println(canvas.getCanvasBuffer());
      savebtn.changeLabel("SAVE", 1, ST77XX_WHITE);
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
      int idx = currcolidx + ((currcolpage - 1) * (COLORPERPAGE));
      if(idx >= NUM_COLORS) idx = 0;
      canvas.draw(x, y, sizes[sizeidx], currColors[currcolidx], idx);
    }
  }
}

void createSizes(int x, int y, int w, int h) {
  int xwidth = w/numsizes;
  sizefield.init(tft, x, y, w, h, ST77XX_WHITE);

  for(int i = 0; i < numsizes; i++) {
    sizebtns[i].init(tft, xwidth * i, y, xwidth, h, ST77XX_WHITE);
    sizebtns[i].changeLabel(String(sizes[i]), 1, ST77XX_BLACK);
  }

  sizebtns[sizeidx].highlight(5, ST77XX_BLACK);
}

void createColors(int x, int y, int w, int h) {
  int xwidth = w/(COLORPERPAGE + 1);
  colorflip.init(tft, x, y, xwidth, h, ST77XX_WHITE);
  colorflip.changeLabel(String(currcolpage), 2, ST77XX_BLACK);
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

bool isTouched(int16_t& x, int16_t& y) {
  SPI.beginTransaction(tsSPISettings);
  TS_Point p;
  bool touched = ts.touched();
  if (touched) {
    p = ts.getPoint();

    // Map raw to screen coordinates
    // Adjust raw range for your specific touch panel
    // Invert axes: low -> high becomes high -> low
    x = map(p.x, 3800, 200, 0, TFT_WIDTH);   // Inverted X
    y = map(p.y, 3800, 200, 0, TFT_HEIGHT);  // Inverted Y

    x = constrain(x, 0, TFT_WIDTH - 1);
    y = constrain(y, 0, TFT_HEIGHT - 1);
  }
  SPI.endTransaction();

  return touched;
}

void waitConnection() {
  tft.setCursor(10, 20);
  tft.println("CONNECTING..\n\n");
  tft.setTextSize(2);
  tft.println("Set WIFI to:");
  tft.print("Name:  ");
  tft.println(WIFI_SSID);
  tft.print("Pass:  ");
  tft.println(WIFI_PASSWORD);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    //serial.print(".");
    delay(300);
  }
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  tft.println("Syncing Time...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1000) {
    delay(100);
    //serial.print(".");
  }
  //serial.println("\nTime synced.");
  tft.println("Time synced.");

  ssl_client.setInsecure();
  ssl_client.setTimeout(1000); // Set connection timeout
  ssl_client.setBufferSizes(4096, 1024);

  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
}

#define CHUNK_MULTIPLIER 5

bool pushDone = true;
bool chunkPushed = false;

// Async callback to track push completion
void processData(AsyncResult &aResult) {
  if (!aResult.isResult()) return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available()) {
    Firebase.printf("Pushed: %s\n", aResult.c_str());
    chunkPushed = true;
  }

  pushDone = true;
}

void sendToFirebase(char* const bigString) {
  size_t len = strlen(bigString);
  size_t offset = 0;
  String sessionid = "";
  int Chunk_size = CHUNK_MULTIPLIER * canvas.getCanvasWidth();

  while(1) {
    app.loop();
    if(app.ready() && pushDone) {
      // Reset flags
      pushDone = false;
      chunkPushed = false;

      sessionid = String(time(nullptr));
      String text = sessionid + " " + String(Chunk_size) + " " + String(canvas.getCanvasWidth()) + " " + String(canvas.getCanvasHeight());

      Database.push<String>(aClient, "/latestsession", text, processData);

      while (!chunkPushed) {
        app.loop();
        delay(1);
      }

      pushDone = true;
      chunkPushed = false;

      break;
    }
  }

  while(1) {
    app.loop();

    if(app.ready() && pushDone) {
      // Reset flags
      pushDone = false;
      chunkPushed = false;
      
      String json = sessionid + " " + String(NUM_COLORS) + " [";
      for (int i = 0; i < NUM_COLORS; i++) {
        json += String(colors[i]);
        if (i < NUM_COLORS - 1) json += ",";
      }
      json += "]";

      String loc = "/colors/" + sessionid;
      Database.set<String>(aClient, loc, json, processData);

      while (!chunkPushed) {
        app.loop();
        delay(1);
      }

      pushDone = true;
      chunkPushed = false;

      break;
    }
  }

  while (offset < len) {
    app.loop();  // keep Firebase client alive and process events

    if (app.ready() && pushDone) {
      pushDone = false;
      chunkPushed = false;
      size_t chunkLen = (len - offset > Chunk_size) ? Chunk_size : (len - offset);

      String escapedChunk = base64EncodeChunk((uint8_t*)(bigString + offset), chunkLen);
      //serial.println("EscapedChunk = " + escapedChunk);
      //serial.println("Length = " + String(escapedChunk.length()));

      String loc = "/sessions/" + sessionid;
      Database.push<String>(aClient, loc, escapedChunk, processData);

      // Wait for confirmation that chunk was pushed
      while (!chunkPushed) {
        app.loop();
        delay(1);
      }

      offset += chunkLen;
    }
  }
}

String base64EncodeChunk(const uint8_t* data, size_t len) {
  const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String output;
  int i = 0;
  while (i < len) {
    uint32_t octet_a = i < len ? data[i++] : 0;
    uint32_t octet_b = i < len ? data[i++] : 0;
    uint32_t octet_c = i < len ? data[i++] : 0;

    uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

    output += base64_chars[(triple >> 18) & 0x3F];
    output += base64_chars[(triple >> 12) & 0x3F];
    output += (i > len + 1) ? '=' : base64_chars[(triple >> 6) & 0x3F];
    output += (i > len) ? '=' : base64_chars[triple & 0x3F];
  }
  return output;
}

