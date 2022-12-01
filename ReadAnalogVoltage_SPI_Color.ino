


/*****************************************************************************

  EANx Analysis with output to an OLED color display

  Reads an analog input on pin, converts it to voltage, grabs a running average 
  of ADC values and and prints the result to the display and debug to Serial Monitor.

  Based on ReadAnalogVoltage script and GFX demo scripts
*****************************************************************************/

// Libraries 
#include <RunningAverage.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_ADS1X15.h>  


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
  #define TFT_CS        4
  #define TFT_RST       3     // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        2
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

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
// Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

// Global Variabls
#define RA_SIZE 20            //Define running average pool size
RunningAverage RA(RA_SIZE);   //Initialize Running Average
  
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

  // Use this initializer (uncomment) if using a 1.3" or 1.54" 240x240 TFT:
  tft.init(240, 240);           // Init ST7789 240x240

  // OR use this initializer (uncomment) if using a 1.69" 280x240 TFT:
  //tft.init(240, 280);           // Init ST7789 280x240

  // OR use this initializer (uncomment) if using a 2.0" 320x240 TFT:
  //tft.init(240, 320);           // Init ST7789 320x240

  // OR use this initializer (uncomment) if using a 1.14" 240x135 TFT:
  //tft.init(135, 240);           // Init ST7789 240x135
  
  // OR use this initializer (uncomment) if using a 1.47" 172x320 TFT:
  //tft.init(172, 320);           // Init ST7789 172x320

  // OR use this initializer (uncomment) if using a 1.9" 170x320 TFT:
  //tft.init(170, 320);           // Init ST7789 170x320

  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  //tft.setSPISpeed(40000000);

  Serial.println(F("Display Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

/*

  // line draw test
  testlines(ST77XX_YELLOW);
  delay(500);

  // optimized lines
  testfastlines(ST77XX_RED, ST77XX_BLUE);
  delay(500);

  testdrawrects(ST77XX_GREEN);
  delay(500);
*/
  //testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  //delay(500);

  tft.fillScreen(ST77XX_BLACK);
  testfillcircles(10, ST77XX_BLUE);
  testdrawcircles(10, ST77XX_WHITE);
  delay(500);

//  testroundrects();
//  delay(500);

//  testtriangles();
//  delay(500);

  tft.fillScreen(ST77XX_GREEN);
  tft.setTextSize(4); 
  tft.setTextColor(ST77XX_BLACK);
  Serial.println("init test done");
  tft.println("init");
  tft.println("complete");
  delay(700);
  tft.fillScreen(ST77XX_BLACK);  
  

  // setup display and calibrate unit
  o2calibration();


  tft.fillScreen(ST77XX_BLACK);

  printLayout();


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
  float multiplier = 0.1875F / 2; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational 
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
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

  currentO2 = ((aveSensorValue * calFactor) * 20.9);  // Units: pct
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
  tft.setCursor(10,10);
  tft.println(F("+++++++++"));
  tft.setTextSize(4);
  tft.println(F("Calibrating"));
  tft.setTextSize(4);
  tft.println(F("O2 Sensor"));
  tft.println(F("++++++++++"));

  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.setGain(GAIN_TWO);           // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  float multiplier = 0.1875F / 2; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  // Check that the ADC is operational 
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  // get running average value from ADC input Pin
  RA.clear();
  for (int x=0; x<= (RA_SIZE*5); x++) {
    int sensorValue = 0;
    sensorValue = ads.readADC_Differential_0_1();
    RA.addValue(sensorValue);
    delay(16);
    // Serial.println(sensorValue);    //mV serial print for debugging
  } 

  calFactor = (1 / RA.getAverage());  // Auto Calibrate to 20.9%

  

}

void printSensorValue()
{
  tft.setCursor(150, 180);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLUE);
  tft.println(aveSensorValue);
}

void deleteSensorValue()
{
  tft.setCursor(150, 180);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevaveSensorValue);
}

void printVoltage()
{
  tft.setCursor(40, 170);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_RED);
  tft.println(voltage,1);
}

void deleteVoltage()
{
  tft.setCursor(40, 170);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevvoltage,1);
}

void printo2()
{
  tft.setCursor(40, 50);
  tft.setTextSize(6);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println(currentO2,1);
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
  tft.setCursor(40, 5);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("O2 %");
  tft.setCursor(20, 130);
  tft.setTextSize(4);
  tft.println("Volts mV");
}

void testlines(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, 0, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, 0, x, tft.height()-1, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, 0, 0, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(0, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(0, tft.height()-1, tft.width()-1, y, color);
    delay(0);
  }

  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, x, 0, color);
    delay(0);
  }
  for (int16_t y=0; y < tft.height(); y+=6) {
    tft.drawLine(tft.width()-1, tft.height()-1, 0, y, color);
    delay(0);
  }
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void testfastlines(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t y=0; y < tft.height(); y+=5) {
    tft.drawFastHLine(0, y, tft.width(), color1);
  }
  for (int16_t x=0; x < tft.width(); x+=5) {
    tft.drawFastVLine(x, 0, tft.height(), color2);
  }
}

void testdrawrects(uint16_t color) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=0; x < tft.width(); x+=6) {
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(ST77XX_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
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

void testtriangles() {
  tft.fillScreen(ST77XX_BLACK);
  uint16_t color = 0xF800;
  int t;
  int w = tft.width()/2;
  int x = tft.height()-1;
  int y = 0;
  int z = tft.width();
  for(t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
}

void testroundrects() {
  tft.fillScreen(ST77XX_BLACK);
  uint16_t color = 100;
  int i;
  int t;
  for(t = 0 ; t <= 4; t+=1) {
    int x = 0;
    int y = 0;
    int w = tft.width()-2;
    int h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
}

