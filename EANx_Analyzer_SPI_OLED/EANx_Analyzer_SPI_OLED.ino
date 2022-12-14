/*****************************************************************************

  EANx Analysis with output to an OLED color display

  Reads an analog input on pin, converts it to voltage, grabs a running average 
  of ADC values and and prints the result to the display and debug to Serial Monitor.

  Based on prior EANx scripts: 
  https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019.git
  https://github.com/ejlabs/arduino-nitrox-analyzer.git

*****************************************************************************/

// Libraries
#include <RunningAverage.h>
#include <SPI.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <Adafruit_ADS1X15.h>
#include <splash.h>
#include "pin_config.h"
#include "OTA.h"

// ST1306 definitions
#define SCREEN_WIDTH 240   // OLED display width, in pixels
#define SCREEN_HEIGHT 240  // OLED display height, in pixels
#define OLED_RESET -1      // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);  //Define OLED display

Adafruit_ADS1115 ads;  // Define ADC - 16-bit version

// Running Average definitions
#define RA_SIZE 20           //Define running average pool size
RunningAverage RA(RA_SIZE);  //Initialize Running Average

// Global Variabls
float prevaveSensorValue = 0;
float aveSensorValue = 0;
float prevvoltage = 0;
float voltage = 0;
float prevO2 = 0;
float currentO2 = 0;
float calFactor = 1;
int modfsw = 0;
int modmsw = 0;
int prevmodfsw = 0;
int prevmodmsw = 0;
float modppo = 1.4;
float multiplier = 0;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  ArduinoOTA.setHostname("EANxDevice");
  setupOTA("EANxDevice", mySSID, myPASSWORD);

  initst7789();
  
  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);

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

  // setup display and calibrate unit

  o2calibration();
  printLayout();
}

// the loop routine runs over and over again forever:
void loop() {

  multiplier = initADC();

  int16_t results;

  results = ads.readADC_Differential_0_1();

  // get running average value from ADC input Pin
  RA.clear();
  for (int x = 0; x <= RA_SIZE; x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    // Serial.println(sensorValue);    //mV serial print for debugging
  }

  // Record old and new ADC values
  ArduinoOTA.handle();
  prevaveSensorValue = aveSensorValue;
  prevO2 = currentO2;
  prevvoltage = voltage;
  prevmodfsw = modfsw;
  prevmodmsw = modmsw;
  aveSensorValue = RA.getAverage();

  currentO2 = (aveSensorValue * calFactor);  // Units: pct
  if (aveSensorValue > 99.9) currentO2 = 99.9;

  voltage = (aveSensorValue * multiplier);  // Units: mV

  modfsw = 33 * ((modppo / (currentO2 / 100)) - 1);
  modmsw = 10 * ((modppo / (currentO2 / 100)) - 1);

  // DEBUG print out the value you read:
  Serial.print("ADC Raw Diff = ");
  Serial.print(aveSensorValue);
  Serial.print("  ");
  Serial.print("Voltage = ");
  Serial.print(voltage);
  Serial.print(" mV");
  Serial.print("  ");
  Serial.print("O2 = ");
  Serial.print(currentO2);
  Serial.print("% ");
  Serial.print(modfsw);
  Serial.println(" FT");

  // Display values on OLED
  if (prevvoltage != voltage) {
    deleteVoltage();
    printVoltage();
  }

  if (prevO2 != currentO2) {
    deleteo2();
    printo2();
  }

  if (prevmodfsw != modfsw) {
    deletemod();
    printmod();
  }
}

void o2calibration() {
  //display "Calibrating"
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0,SCREEN_HEIGHT*.1);
  tft.println(F("++++++++++"));
  tft.println();
  tft.println(F("Calibrating"));
  tft.println(F("O2 Sensor"));
  tft.println();
  tft.println(F("++++++++++"));
  Serial.println("Calibration Screen Text");

  initADC();

  Serial.println("Post ADS check statement");
  // get running average value from ADC input Pin
  RA.clear();
  for (int x = 0; x <= (RA_SIZE * 5); x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    Serial.print("calibrating ");
    Serial.println(sensorValue);  //mV serial print for debugging
  }
  tft.fillScreen(ST77XX_BLACK);
  calFactor = (1 / RA.getAverage() * 20.900);  // Auto Calibrate to 20.9%
}

void printmod() {
  tft.setCursor(SCREEN_WIDTH*.5, SCREEN_HEIGHT*.8);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print(modfsw);
  tft.print(" FT");
  tft.setCursor(SCREEN_WIDTH*.55, SCREEN_HEIGHT*.9);
  tft.print(modmsw);
  tft.println(" m");
}

void deletemod() {
  tft.setCursor(SCREEN_WIDTH*.5, SCREEN_HEIGHT*.8);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_BLACK);
  tft.print(prevmodfsw);
  tft.print(" FT");
  tft.setCursor(SCREEN_WIDTH*.55, SCREEN_HEIGHT*.9);
  tft.print(prevmodmsw);
  tft.println(" m");
}

void printVoltage() {
  tft.setCursor(SCREEN_WIDTH*.20, SCREEN_HEIGHT*.8);
  tft.setTextSize(1);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ST77XX_RED);
  tft.println(voltage, 1);
}

void deleteVoltage() {
  tft.setCursor(SCREEN_WIDTH*.20, SCREEN_HEIGHT*.8);
  tft.setTextSize(1);
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevvoltage, 1);
}

void printo2() {
  tft.setCursor(SCREEN_WIDTH*.15, SCREEN_HEIGHT*.5);
  tft.setTextSize(2);
  tft.setFont(&FreeSans24pt7b);
  if (currentO2 > 20 and currentO2 < 22) { tft.setTextColor(ST77XX_CYAN, ST77XX_BLACK); }
  if (currentO2 < 20) { tft.setTextColor(ST77XX_RED, ST77XX_BLACK); }
  if (currentO2 > 22) { tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK); }
  tft.println(currentO2, 1);
}

void deleteo2() {
  tft.setCursor(SCREEN_WIDTH*.15, SCREEN_HEIGHT*.5);
  tft.setTextSize(2);
  tft.setFont(&FreeSans24pt7b);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevO2, 1);
}

void printLayout() {
  tft.setCursor(SCREEN_WIDTH*.35, SCREEN_HEIGHT*.15);
  tft.setTextSize(1);
  tft.setFont(&FreeSans24pt7b);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("O2 %");
  tft.setFont(&FreeSans18pt7b);
  tft.setTextColor(ST77XX_BLUE);
  tft.setCursor(SCREEN_WIDTH*.15, SCREEN_HEIGHT*.7);
  tft.print("mV");
  tft.setTextColor(ST77XX_ORANGE);
  tft.setCursor(SCREEN_WIDTH*.5, SCREEN_HEIGHT*.7);
  tft.println("MOD");
}


void initst7789() {
  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT);  // Init ST7789 240x240
                                          //  tft.init(128, 160);       // Init ST7735 128x160
  tft.setRotation(2);                     // Adjust SS7789 Orientation
}

float initADC() {
  // init ADC and Set gain

  // The ADC input range (or gain) can be changed via the following
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);  // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  float multiplier = 0.0625; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    tft.fillScreen(ST77XX_YELLOW);
    tft.setCursor(0, 30);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.println(F("Error"));
    tft.println(F("No ADC"));
    while (1)
      ;
  }
  return (multiplier);
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = radius; x < tft.width(); x += radius * 2) {
    for (int16_t y = radius; y < tft.height(); y += radius * 2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = 0; x < tft.width() + radius; x += radius * 2) {
    for (int16_t y = 0; y < tft.height() + radius; y += radius * 2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}
