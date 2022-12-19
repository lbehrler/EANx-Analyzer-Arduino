/*****************************************************************************
  EANx Analysis with output to an SSD 1306 OLED mono display
  Reads an analog input on pin, converts it to voltage, grabs a running average 
  of ADC values and and prints the result to the display and debug to Serial Monitor.
  Based on prior EANx scripts: 
  https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019.git
  https://github.com/ejlabs/arduino-nitrox-analyzer.git
*****************************************************************************/

// Libraries 
//#include <Arduino.h>
#include <RunningAverage.h>
#include <Adafruit_ADS1X15.h>  
#include <Wire.h>
#include <Adafruit_SSD1306.h> // Hardware-specific library for ST1306
#include "pin_config.h"

// ST1306 definitions
#define SCREEN_WIDTH  128             // OLED display width, in pixels
#define SCREEN_HEIGHT  64             // OLED display height, in pixels
#define OLED_RESET     -1             // Reset pin # (or -1 if sharing Arduino reset pin)

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D, 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADS1115 ads;  // Define ADC - 16-bit version 

// Running Average definitions
#define RA_SIZE 20            //Define running average pool size
RunningAverage RA(RA_SIZE);   //Initialize Running Average

// Global Variables 
       
float aveSensorValue = 0;
float sensorValue = 0;
float voltage = 0;
float prevO2 = 0;
float currentO2 = 0;
float calFactor = 1;
int modfsw = 0;
int modmsw = 0;
float modppo = 1.4;
float multiplier = 0;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  initst1306();

  // Display Text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println("Startup");
  display.display();

  Serial.println(F("Display Initialized"));

  display.clearDisplay();
  display.display();

  o2calibration();

}

// the loop routine runs over and over again forever:
void loop() {
 
  multiplier = initADC();

  // get running average value from ADC input Pin
  RA.clear();
  for (int x=0; x<= RA_SIZE; x++) {
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    // Serial.println(sensorValue);    //mV serial print for debugging
  } 

  // Record old and new ADC values 
  prevO2 = currentO2;
  aveSensorValue = RA.getAverage();

  currentO2 = (aveSensorValue * calFactor);  // Units: pct
  if (currentO2 > 99.9) currentO2 = 99.9;

  voltage = (aveSensorValue * multiplier);  // Units: mV

  modfsw =  33 * ((modppo / (currentO2 / 100)) - 1);
  modmsw =  10 * ((modppo / (currentO2 / 100)) - 1);

  delay(300);
  // DEBUG print out the value you read:
  Serial.print(F("ADC Raw Diff = "));
  Serial.print(aveSensorValue);
  Serial.print(F("  "));
  Serial.print(F("Voltage = ")); 
  Serial.print(voltage);
  Serial.print(F(" mV"));
  Serial.print(F("  "));
  Serial.print(F("O2 = "));
  Serial.print(currentO2);
  Serial.print(F(" % "));
  Serial.print(modfsw);
  Serial.println(F(" FT")); 

  if( prevO2!=currentO2)
  {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.println(F("EANx O2 %"));
  display.setTextSize(2);
  display.setCursor(0,20);
  display.print(F("O2% "));
  display.setTextSize(3);
  display.println(currentO2,1);
  display.setCursor(0,45);
  display.setTextSize(2);
  display.print(F("MOD "));
  display.setTextSize(1);
  display.setCursor(55,45);
  display.print(modfsw);
  display.println(F(" FT"));
  display.setCursor(55,55);
  display.print(modmsw);
  display.println(F(" m"));
  display.display();
  }

}

void o2calibration() 
{
  //display "Calibrating"
  display.fillScreen(BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println(F("Init / Cal"));
  display.setTextSize(2);
  display.setCursor(0,20);
    display.println(F("O2 Sensor"));
  Serial.println(F("Calibration Screen Text"));
  display.display();
  delay(200);

  initADC();

  Serial.println(F("Post ADS check statement"));
  // get running average value from ADC input Pin
  RA.clear();
  for (int x=0; x<= (RA_SIZE*5); x++) {
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    Serial.print(F("calibrating "));
    Serial.println(sensorValue);    //mV serial print for debugging
  } 
  display.clearDisplay();
  display.display();
  calFactor = (1 / RA.getAverage()*20.900);  // Auto Calibrate to 20.9%
}


void initst1306()
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  // Clear the buffer
  display.clearDisplay();
  display.display();
}

float initADC()
{
    // init ADC 
  //Serial.println(F("Getting differential reading from AIN0 (P) and AIN1 (N)"));

  // The ADC input range (or gain) can be changed via the following
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  multiplier = 0.0625; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational 
  if (!ads.begin()) {
    Serial.println(F("Failed to initialize ADS."));
    display.println(F("Error"));
    display.println(F("No Init"));
    display.display();
    while (1);
  }

  return (multiplier);
}

