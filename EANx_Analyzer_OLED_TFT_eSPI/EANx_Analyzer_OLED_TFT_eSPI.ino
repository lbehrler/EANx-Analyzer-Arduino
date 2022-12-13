

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
//#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <TFT_eSPI.h>
#include <Adafruit_ADS1X15.h>
#include <splash.h>
#include "pin_config.h"
//#include "OTA.h"
#include "C:\Users\Brian E\Documents\Arduino\libraries\TFT_eSPI\User_Setup.h"


// ST1306 definitions
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 160  // OLED display height, in pixels
#define OLED_RESET -1      // Reset pin # (or -1 if sharing Arduino reset pin)

// Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);  //Define OLED display
TFT_eSPI tft = TFT_eSPI(); 

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
  Serial.begin(115200);
  //ArduinoOTA.setHostname("EANxTinyPico");
  //setupOTA("EANxTinyPico", mySSID, myPASSWORD);

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  Serial.println("Display Initialized");

  tft.fillScreen(TFT_BLACK);
  testfillcircles(5, TFT_BLUE);
  testdrawcircles(5, TFT_WHITE);
  delay(500);

  tft.fillScreen(TFT_GREEN);
  tft.setTextSize(1);
  tft.setTextColor(TFT_BLACK);
  Serial.println("init display test done");
  tft.drawString("display", 0, 0, 4);
  tft.drawString("init", 0, 20, 4);
  tft.drawString("complete", 0, 40, 4);
  delay(500);
  tft.fillScreen(TFT_BLACK);

  // setup display and calibrate unit

  o2calibration();
  printLayout();
}

// the loop routine runs over and over again forever:
void loop() {

  multiplier = initADC();

  int16_t results;

  results = ads.readADC_Differential_0_1();

  Serial.print("Differential: ");
  Serial.print(results);
  Serial.print("(");
  Serial.print(results * multiplier / 2);
  Serial.println("mV)");

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
  //ArduinoOTA.handle();
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

  if (prevO2 != currentO2) {
  if (currentO2 > 20 and currentO2 < 22) { tft.setTextColor(TFT_CYAN, TFT_BLACK); }
  if (currentO2 < 20) { tft.setTextColor(TFT_RED, TFT_BLACK); }
  if (currentO2 > 22) { tft.setTextColor(TFT_GREEN, TFT_BLACK); }
  tft.setTextSize(1);
  tft.drawFloat(currentO2, 1, SCREEN_WIDTH*.1, SCREEN_HEIGHT*.2, 7);
  tft.setTextSize(1);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(voltage, 1, SCREEN_WIDTH*.2, SCREEN_HEIGHT*.65, 2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawNumber(modfsw, SCREEN_WIDTH*.5, SCREEN_HEIGHT*.65, 2);
  tft.drawString(" FT", SCREEN_WIDTH*.75, SCREEN_HEIGHT*.65, 2);
  tft.drawNumber(modmsw, SCREEN_WIDTH*.5, SCREEN_HEIGHT*.75, 2);
  tft.drawString(" m", SCREEN_WIDTH*.75, SCREEN_HEIGHT*.75, 2);
  }
}

void o2calibration() {
  //display "Calibrating"
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("+++++++++++++", 0, SCREEN_HEIGHT*.10, 4);
  tft.drawString("Calibrating", 0, SCREEN_HEIGHT*.30, 4);
  tft.drawString("O2 Sensor", 0, SCREEN_HEIGHT*.60, 4);
  tft.drawString("+++++++++++++", 0, SCREEN_HEIGHT*.80, 4);
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
  tft.fillScreen(TFT_BLACK);
  calFactor = (1 / RA.getAverage() * 20.900);  // Auto Calibrate to 20.9%
}

void printLayout() {
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("O2 %",SCREEN_WIDTH*.3, SCREEN_HEIGHT*.05, 4);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("mV",SCREEN_WIDTH*.20, SCREEN_HEIGHT*.55, 2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.drawString("MOD",SCREEN_WIDTH*.60, SCREEN_HEIGHT*.55, 2);
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
  //  tft.setTextSize(4);
    tft.setTextColor(TFT_RED);
    tft.drawString("Error", 0, 0, 2);
    tft.drawString("No Init", 0, 20, 2);
    delay(5000);
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