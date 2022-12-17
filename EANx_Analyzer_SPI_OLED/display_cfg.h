// STXXXX definitions
#define SCREEN_WIDTH  240  // OLED display width, in pixels with display in portrait orientation
#define SCREEN_HEIGHT 240  // OLED display height, in pixels with display in portrait orientation
#define OLED_RESET -1      // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);  //Define OLED display

void initst7789() {
  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT);  // Init ST7789 240x240
  tft.setRotation(0);                     // Adjust SS7789 Orientation

  Serial.println("Display Initialized");  
  tft.setCursor(0, SCREEN_HEIGHT*.2);
  tft.fillScreen(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.setFont(&FreeSans18pt7b);
  tft.setTextColor(ST77XX_BLACK);
  Serial.println("init display test done");
  tft.println("display");
  tft.println("init");
  tft.println("complete");
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
}