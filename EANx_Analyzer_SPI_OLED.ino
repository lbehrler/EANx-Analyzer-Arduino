

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
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST1306
#include <Adafruit_ADS1X15.h>  
#include <Adafruit_SSD1306.h>
#include <splash.h>

// Chip Specific settings for SPI OLED 
#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#elif defined(SEEED_XIAO_M0)  // Seeed XAIO
  #define TFT_CS        6
  #define TFT_RST       2     // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        7
  #define TFT_MOSI      10    // Data out
  #define TFT_SCLK      8     // Clock out
  #define ADCPIN0       0
  #define ADCPIN1       1
  #define ADCFACT       1024  

#elif defined(ARDUINO_ESP32_PICO)
  #define TFT_SDA       21     
  #define TFT_SCL       22
  #define TFT_MOSI      23    // Data out
  #define TFT_SCLK      18    // Clock out  #define 
  #define TFT_RST       5     // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        10
  #define TFT_CS        9
  #define ADCPIN0       36
  #define ADCPIN1       39
  #define ADCFACT       4095 

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST       9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        8
  #define ADCPIN0       0
  #define ADCPIN1       3
#endif

// OLED definitions
#define SCREEN_WIDTH  128             // OLED display width, in pixels
#define SCREEN_HEIGHT 32              // OLED display height, in pixels
#define OLED_RESET     -1              // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST); //Define OLED display

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(19200);

  initst7789();
  initst1306();

  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  //tft.setSPISpeed(40000000);

  Serial.println("Display Initialized");

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);

  tft.fillScreen(ST77XX_GREEN);
  tft.setTextSize(4); 
  tft.setTextColor(ST77XX_BLACK);
  Serial.println("init display test done");
  tft.println("init");
  tft.println("display");
  tft.println("complete");
  delay(500);
  tft.fillScreen(ST77XX_BLACK);  

  display.clearDisplay();
  display.display();

  // Display Text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.println("Intialized");
  display.display();
  delay(500);
  display.clearDisplay();
  
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
}


// the loop routine runs over and over again forever:
void loop() {
 
  // init ADC 
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
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
    tft.setCursor(0,30);
    tft.setTextSize(4);
    tft.setTextColor(ST77XX_RED);
    tft.println(F("Error"));
    tft.println(F("No Init"));
    while (1);
  }

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
  aveSensorValue = RA.getAverage();

  currentO2 = (aveSensorValue * calFactor);  // Units: pct
  if (aveSensorValue > 99.9) currentO2 = 99.9;

  voltage = (aveSensorValue * multiplier);  // Units: mV

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
  Serial.println(" %");

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

  if( prevaveSensorValue!=aveSensorValue)
  {
    deleteSensorValue();
    printSensorValue();
  }
 




}
/*************************************************************************************************************************************************************/
void o2calibration() 
{
  //display "Calibrating"
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(4);
  tft.setCursor(0,10);
  tft.println(F("++++++++++"));
  tft.println();
  tft.setTextSize(3);
  tft.println(F("Calibrating"));
  tft.println();
  tft.setTextSize(4);
  tft.println(F("O2 Sensor"));
  tft.println();
  tft.println(F("++++++++++"));
  Serial.println("Calibration Screen Text");
  
  testscrolltext();

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
  float multiplier =  0.0625;   /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational 
  if (!ads.begin()) 
  {
    tft.fillScreen(ST77XX_RED);
    tft.setCursor(0,30);
    tft.setTextSize(4);
    tft.setTextColor(ST77XX_BLACK);
    tft.println("Err");
    tft.println("No ADC Init");
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
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
  tft.fillScreen(ST77XX_BLACK);
  calFactor = (1 / RA.getAverage()*20.900);  // Auto Calibrate to 20.9%

  

}

void printSensorValue()
{
  tft.setCursor(130, 165);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println(aveSensorValue);
}

void deleteSensorValue()
{
  tft.setCursor(130, 165);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevaveSensorValue);
}

void printVoltage()
{
  tft.setCursor(30, 160);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_RED);
  tft.println(voltage,1);
}

void deleteVoltage()
{
  tft.setCursor(30, 160);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevvoltage,1);
}

void printo2()
{
  tft.setCursor(40, 50);
  tft.setTextSize(6);
  if(currentO2>20 and currentO2<22) {  tft.setTextColor(ST77XX_CYAN);}
  if(currentO2<20) {  tft.setTextColor(ST77XX_RED);}
  if(currentO2>22) {  tft.setTextColor(ST77XX_GREEN);}
  tft.println(currentO2,1);
    // Display Text on ST1306
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.print("O2% ");
  display.setTextSize(3);
  display.println(currentO2,1);
  display.display();
  delay(500);
  display.clearDisplay();
}

void deleteo2()
{
  tft.setCursor(40, 50);
  tft.setTextSize(6);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevO2,1);
}

void printLayout()
{
  tft.setCursor(50, 5);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("O2 %");
  tft.setCursor(30, 120);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_BLUE);
  tft.print("mV");
  tft.setTextColor(ST77XX_ORANGE);
  tft.println("  Raw");
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

void initst7789()
{
  tft.init(240, 240);           // Init ST7789 240x240
  tft.setRotation(2);       // Adjust SS7789 Orientation
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=radius; x < tft.width(); x+=radius*2) {
    for (int16_t y=radius; y < tft.height(); y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x=0; x < tft.width()+radius; x+=radius*2) {
    for (int16_t y=0; y < tft.height()+radius; y+=radius*2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
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
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(500);
  display.stopscroll();
  delay(500);
}
