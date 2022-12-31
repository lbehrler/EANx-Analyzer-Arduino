/*****************************************************************************

  EANx Analysis with output to an OLED color display

  Reads an analog input from an ADC, converts it to mV, and creates a running average 
  of ADC values and and prints the result to the display and debug to Serial Monitor.

  Based on prior EANx scripts: 
  https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019.git
  https://github.com/ejlabs/arduino-nitrox-analyzer.git

*****************************************************************************/

// Libraries
#include <Wire.h>
#include <RunningAverage.h>
#include <SPI.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <TFT_eSPI.h>
#include <Adafruit_ADS1X15.h>
#include <splash.h>
#include "pin_config.h"

// display definitions
#define TFT_WIDTH  240   // OLED display width, in pixels
#define TFT_HEIGHT 240   // OLED display height, in pixels
#define ResFact       2    // 1 = 128x128   2 = 240x240

TFT_eSPI tft = TFT_eSPI();

Adafruit_ADS1115 ads;  // Define ADC - 16-bit version

// Running Average definitions
#define RA_SIZE 20           //Define running average pool size
RunningAverage RA(RA_SIZE);  //Initialize Running Average

// Global Variabls
float prevaveSensorValue = 0;
float aveSensorValue = 0;
float mVolts = 0;
float batVolts = 0;
float prevO2 = 0;
float currentO2 = 0;
float calFactor = 1;
int modfsw = 0;
int modmsw = 0;
float modppo = 1.4;
float multiplier = 0;


const int buttonPin=BUTTON_PIN; // push button

void setup() {
  Serial.begin(115200);

 //OTA
//ArduinoOTA.setHostname(OTADEVICE);
//setupOTA(OTADEVICE, mySSID, myPASSWORD);}

  pinMode(buttonPin,INPUT_PULLUP);  

  //setup TFT
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  Serial.println("Display Initialized");

  tft.fillScreen(TFT_BLACK);
  testfillcircles(5, TFT_BLUE);
  testdrawcircles(5, TFT_WHITE);
  delay(500);

  tft.fillScreen(TFT_GREEN);
  tft.setTextSize(1 * ResFact);
  tft.setTextColor(TFT_BLACK);
  Serial.println("init display test done");
  tft.drawString("display", 0, 0, 4);
  tft.drawString("init", 0, 30, 4);
  tft.drawString("complete", 0, 60, 4);
  delay(500);
  tft.fillScreen(TFT_BLACK);

  // setup display and calibrate unit
  o2calibration();
  safetyrule();
  printLayout();
}

// the loop routine runs over and over again forever:
void loop() {

// Enable for OTA
// ArduinoOTA.handle();

  multiplier = initADC();

  int bstate = digitalRead(buttonPin);
  // Serial.println(bstate);
  if (bstate == LOW) {
    o2calibration();
    safetyrule();
    printLayout(); }

  // get running average value from ADC input Pin
  RA.clear();
  for (int x = 0; x <= RA_SIZE; x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    //Serial.println(sensorValue);    //mV serial print for debugging
  }
  delay(100); // slowing down loop a bit 

  // Record old and new ADC values
  prevaveSensorValue = aveSensorValue;
  prevO2 = currentO2;
  aveSensorValue = RA.getAverage();

  currentO2 = (aveSensorValue * calFactor);  // Units: pct
  if (currentO2 > 99.9) currentO2 = 99.9;

  mVolts = (aveSensorValue * multiplier);  // Units: mV

  modfsw = 33 * ((modppo / (currentO2 / 100)) - 1);
  modmsw = 10 * ((modppo / (currentO2 / 100)) - 1);

  // DEBUG print out the value you read:
  Serial.print("ADC Raw Diff = ");
  Serial.print(aveSensorValue);
  Serial.print("  ");
  Serial.print("Sensor mV = ");
  Serial.print(mVolts);
  Serial.print(" mV ");
  Serial.print("Batt V = ");
  Serial.print(batVolts);
  Serial.print(" V ");
  Serial.print("O2 = ");
  Serial.print(currentO2);
  Serial.print("% ");
  Serial.print(modfsw);
  Serial.println(" FT");

  if (prevO2 != currentO2) {
  if (currentO2 > 20 and currentO2 < 22) { tft.setTextColor(TFT_CYAN, TFT_BLACK); }
  if (currentO2 < 20) { tft.setTextColor(TFT_RED, TFT_BLACK); }
  if (currentO2 > 22) { tft.setTextColor(TFT_GREEN, TFT_BLACK); }

  // Draw Text -- Adjust these layouts to suit you LCD
  tft.setTextSize(1 * ResFact);
  String o2 = String (currentO2, 1);
  tft.drawCentreString(o2, TFT_WIDTH*.5, TFT_HEIGHT*.2, 7);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  String mv = String(mVolts, 1);
  tft.drawString(String(mv + " mV  "), TFT_WIDTH*.1, TFT_HEIGHT*.72, 2);
  String bv = String(batVolts, 1);
  tft.drawString(String(bv + " V  "), TFT_WIDTH*.1, TFT_HEIGHT*.83, 2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_PURPLE, TFT_BLACK);
  tft.drawCentreString(String(millis()/1000), TFT_WIDTH*.5, TFT_HEIGHT*.90, 2);
  tft.setTextSize(1 * ResFact);  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  String modf = String(modfsw);
  tft.drawString(String(modf + " FT  "), TFT_WIDTH*.6, TFT_HEIGHT*.72, 2);
  String modm = String(modmsw);
  tft.drawString(String(modm + " m  "), TFT_WIDTH*.6, TFT_HEIGHT*.83, 2);
  }
}

void o2calibration() {
  //display "Calibrating"
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1 * ResFact);
  tft.drawString("+++++++++++++", TFT_WIDTH*.1, TFT_HEIGHT*.10, 2);
  tft.drawString("Calibrating", TFT_WIDTH*.1, TFT_HEIGHT*.30, 2);
  tft.drawString("O2 Sensor", TFT_WIDTH*.1, TFT_HEIGHT*.60, 2);
  tft.drawString("+++++++++++++", TFT_WIDTH*.1, TFT_HEIGHT*.80, 2);
  Serial.println("Calibration Screen Text");

  batVolts = (batStat() / 1000)*BAT_ADJ; //Battery Check ESP based boards 

  initADC();

  Serial.println("Post ADS check statement");
  // get running average value from ADC input Pin
  RA.clear();
  for (int x = 0; x <= (RA_SIZE * 3); x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    Serial.print("calibrating ");
    Serial.println(sensorValue);  //raw sensor serial print for debugging
  }
  tft.fillScreen(TFT_BLACK);
  calFactor = (1 / RA.getAverage() * 20.900);  // Auto Calibrate to 20.9%
  // Serial.println(calFactor);  // average cal factor 
}

// Draw Layout -- Adjust this layouts to suit you LCD
void printLayout() {
  tft.setTextSize(1 * ResFact);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawCentreString("O2 %",TFT_WIDTH*.50, TFT_HEIGHT*.01, 4);
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("Info",TFT_WIDTH*.10, TFT_HEIGHT*.6, 2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("MOD",TFT_WIDTH*.60, TFT_HEIGHT*.6, 2);
}

float initADC() {
  // init ADC and Set gain

  // The ADC input range (or gain) can be changed via the following
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);         // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  float multiplier = 0.0625; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    tft.fillScreen(TFT_YELLOW);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1 * ResFact);
    tft.drawString("Error", 0, 0, 4);
    tft.drawString("No ADC", 0, 40, 4);
    delay(5000);
    while (1);  
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

void safetyrule()  {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1 * ResFact);
  randomSeed(millis());
  int randNumber = random(5);
  Serial.println(randNumber);
  if (randNumber == 0) {
    tft.drawCentreString("Seek proper", TFT_WIDTH*.5, TFT_HEIGHT*.10, 2);
    tft.drawCentreString("training", TFT_WIDTH*.5, TFT_HEIGHT*.20, 2); }
  else if (randNumber == 1){
    tft.drawCentreString("Maintain a", TFT_WIDTH*.5, TFT_HEIGHT*.10, 2);
    tft.drawCentreString("continious", TFT_WIDTH*.5, TFT_HEIGHT*.20, 2);
    tft.drawCentreString("guideline to", TFT_WIDTH*.5, TFT_HEIGHT*.30, 2);
    tft.drawCentreString("the surface", TFT_WIDTH*.5, TFT_HEIGHT*.40, 2); }
  else if (randNumber == 2){
    tft.drawCentreString("Stay within", TFT_WIDTH*.5, TFT_HEIGHT*.10, 2);
    tft.drawCentreString("your depth", TFT_WIDTH*.5, TFT_HEIGHT*.20, 2);
    tft.drawCentreString("limitations", TFT_WIDTH*.5, TFT_HEIGHT*.30, 2); }
  else if (randNumber == 3){
    tft.drawCentreString("Proper gas", TFT_WIDTH*.5, TFT_HEIGHT*.10, 2);
    tft.drawCentreString("management", TFT_WIDTH*.5, TFT_HEIGHT*.20, 2); }
  else{
    tft.drawCentreString("Use appropriate", TFT_WIDTH*.5, TFT_HEIGHT*.10, 2);
    tft.drawCentreString("properly maintaned", TFT_WIDTH*.5, TFT_HEIGHT*.20, 2);
    tft.drawCentreString("equipment", TFT_WIDTH*.5, TFT_HEIGHT*.30, 2); }
  delay (3000);
  tft.fillScreen(TFT_BLACK);
}