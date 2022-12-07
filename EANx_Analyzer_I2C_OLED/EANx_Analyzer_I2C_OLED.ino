/*****************************************************************************

  EANx Analysis with output to an SSD 1306 OLED mono display

  Reads an analog input on pin, converts it to voltage, grabs a running average 
  of ADC values and and prints the result to the display and debug to Serial Monitor.

  Based on prior EANx scripts: 
  https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019.git
  https://github.com/ejlabs/arduino-nitrox-analyzer.git

*****************************************************************************/

// Libraries 
#include <Arduino.h>
#include <U8g2lib.h>
#include <RunningAverage.h>
#include <Adafruit_ADS1X15.h>  

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//#include <SPI.h>
//#include <Adafruit_GFX.h>    // Core graphics library

//#include <Adafruit_SSD1306.h> // Hardware-specific library for ST1306
//#include <splash.h>
#include "pin_config.h"

// ST1306 definitions
#define SCREEN_WIDTH  128             // OLED display width, in pixels
#define SCREEN_HEIGHT 64              // OLED display height, in pixels
#define OLED_RESET     -1             // Reset pin # (or -1 if sharing Arduino reset pin)

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Adafruit_ADS1115 ads;  // Define ADC - 16-bit version 

// Running Average definitions
#define RA_SIZE 20            //Define running average pool size
RunningAverage RA(RA_SIZE);   //Initialize Running Average

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
/* sample 
  u8g2.setFont(u8g2_font_unifont_t_chinese2);  // use chinese2 for all the glyphs of "你好世界"
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("Hello World!");
  u8g2.setCursor(0, 40);
  u8g2.print("你好世界");		// Chinese "Hello World" 
  u8g2.sendBuffer();   */

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);


  u8g2.begin();  
  
  u8g2.setFont(u8g2_font_inb30_mr);	// set the target font to calculate the pixel width
  width = u8g2.getUTF8Width(text);		// calculate the pixel width of the text
  
  u8g2.setFontMode(0);		// enable transparent mode, which is faster


  initst1306();

  // Display Text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println("Startup");
  display.display();

  Serial.println("Display Initialized");

  display.clearDisplay();
  display.display();

  display.fillScreen(WHITE);
  display.setTextSize(3); 
  display.setTextColor(BLACK);
  Serial.println("init display test done");
  display.setCursor(10, 0);
  display.println("init");
  display.println("display");
  display.println("complete");
  display.display();
  delay(500);
  display.clearDisplay();
  display.display();
  
  // setup display and calibrate unit
  
  o2calibration();
  printLayout();

  // Display Text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println("O2 Calibrated");
  display.display();
  delay(500);
  display.clearDisplay();
  display.display();
}

// the loop routine runs over and over again forever:
void loop() {
 
  multiplier = initADC();

  int16_t results;

  results = ads.readADC_Differential_0_1();

  Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(results * multiplier / 2); Serial.println("mV)");

  // get running average value from ADC input Pin
  RA.clear();
  for (int x=0; x<= RA_SIZE; x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    // Serial.println(sensorValue);    //mV serial print for debugging
  } 

  // Record old and new ADC values 
  prevaveSensorValue = aveSensorValue;
  prevO2 = currentO2;
  prevvoltage = voltage;
  prevmodfsw = modfsw;
  prevmodmsw = modmsw;
  aveSensorValue = RA.getAverage();

  currentO2 = (aveSensorValue * calFactor);  // Units: pct
  if (aveSensorValue > 99.9) currentO2 = 99.9;

  voltage = (aveSensorValue * multiplier);  // Units: mV

  modfsw =  33 * ((modppo / (currentO2 / 100)) - 1);
  modmsw =  10 * ((modppo / (currentO2 / 100)) - 1);

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
  Serial.print(" %");
  Serial.print(modfsw);
  Serial.println(" FT");

  // Display values on OLED 
  if( prevvoltage!=voltage)
  {
    deleteVoltage();
    printVoltage();
  }
  
  if( prevO2!=currentO2)
  {
    deleteo2();
    printo2();
  }

//  if( prevaveSensorValue!=aveSensorValue)
//  {
//    deleteSensorValue();
//    printSensorValue();
//  }

  if( prevmodfsw!=modfsw)
  {
    deletemod();
    printmod();
  }

}

void o2calibration() 
{
  //display "Calibrating"
  display.fillScreen(BLACK);
  display.setTextColor(WHITE);
  display.setTextSize(4);
  display.setCursor(0,10);
  display.println(F("++++++++++"));
  display.println();
  display.setTextSize(3);
  display.println(F("Calibrating"));
  display.println();
  display.setTextSize(4);
  display.println(F("O2 Sensor"));
  display.println();
  display.println(F("++++++++++"));
  Serial.println("Calibration Screen Text");
  display.display();
  testscrolltext();

  initADC();

  Serial.println("Post ADS check statement");
  // get running average value from ADC input Pin
  RA.clear();
  for (int x=0; x<= (RA_SIZE*5); x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    Serial.print("calibrating ");
    Serial.println(sensorValue);    //mV serial print for debugging
  } 
  display.clearDisplay();
  display.display();
  calFactor = (1 / RA.getAverage()*20.900);  // Auto Calibrate to 20.9%

}

void printSensorValue()
{
  display.setCursor(130, 165);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println(aveSensorValue);
}

void deleteSensorValue()
{
  display.setCursor(130, 165);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.println(prevaveSensorValue);
}

void printmod()
{
  display.setCursor(130, 165);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(modfsw);
  display.print(" FT");
  display.setCursor(130, 185);
  display.print(modmsw);
  display.println(" m");
}

void deletemod()
{
  display.setCursor(130, 165);
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.print(prevmodfsw);
  display.print(" FT");
  display.setCursor(130, 185);
  display.print(prevmodmsw);
  display.println(" m");
}

void printVoltage()
{
  display.setCursor(30, 160);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.println(voltage,1);
}

void deleteVoltage()
{
  display.setCursor(30, 160);
  display.setTextSize(4);
  display.setTextColor(BLACK);
  display.println(prevvoltage,1);
}

void printo2()
{
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.print("O2% ");
  display.setTextSize(3);
  display.setCursor(45,5);
  display.println(currentO2,1);
  display.display();
}

void deleteo2()
{
  display.clearDisplay();
  display.display();
}

void printLayout()
{
  display.setCursor(50, 5);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.println("O2 %");
  display.setCursor(30, 120);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.print("mV");
  display.setTextColor(WHITE);
  display.println("  MOD");
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
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

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
  float multiplier = 0.0625; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational 
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    display.println(F("Error"));
    display.println(F("No Init"));
    display.display();
    while (1);
  }

  return (multiplier);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("Calibrate O2"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(500);
  display.stopscroll();
  delay(500);
  display.startscrollleft(0x00, 0x0F);
  delay(500);
  display.stopscroll();
  delay(500);
  display.startscrolldiagright(0x00, 0x07);
  delay(500);
  display.startscrolldiagleft(0x00, 0x07);
  delay(500);
  display.stopscroll();
  delay(500);
  display.clearDisplay();
  display.display();
}
