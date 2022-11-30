
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
#include <u8g2>

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
  #define ADCPIN        0

#elif defined(ARDUINO_ESP32_PICO)
  #define TFT_SCL       22
  #define TFT_SDA       21    
  #define TFT_MOSI      23    // Data out
  #define TFT_SCLK      18    // Clock out  #define 
  #define TFT_RST       5                                            
  #define TFT_DC        10
  #define TFT_CS        9
  #define ADCPIN        36  

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST       9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        8
  #define ADCPIN        0
#endif


Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


// Global Variabls
#define RA_SIZE 20            //Define running average pool size
RunningAverage RA(RA_SIZE);   //Initialize Running Average

int prevaveSensorValue = 0;         
int aveSensorValue = 0;
float prevvoltage = 0;
float voltage = 0;

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
  Serial.println(F("Initialized"));

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
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);  

  
  displaycalibration();
  printLayout();
  //canvas.setCursor(5, 5);
  //canvas.setTextColor(ST77XX_YELLOW, ST77XX_BLACK);
 // canvas.setTextSize(5);
 // canvas.println(aveSensorValue);
 // tft.drawBitmap(40, 120, canvas.getBuffer(), 128, 64, ST77XX_BLUE, ST77XX_BLACK); // Copy to screen
}

// the loop routine runs over and over again forever:
void loop() {

  // get running average value from ADC input Pin
  RA.clear();
  for (int x=0; x<= RA_SIZE; x++) {
    int sensorValue = 0;
    sensorValue = analogRead(ADCPIN);
    RA.addValue(sensorValue);
    delay(16);
    Serial.println(sensorValue);    //mV serial print for debugging
  } 

  // Record old and new ADC values 
  prevaveSensorValue = aveSensorValue;
  prevvoltage = voltage;
  aveSensorValue = RA.getAverage();

  // Convert the analog reading to voltage (0 - 1023) Seeed XIAO (0 - 4095) ESP32

  voltage = ((aveSensorValue * 3.3) / 4095);

  // DEBUG print out the value you read:
  Serial.print("ADC Value = ");
  Serial.print(aveSensorValue);
  Serial.print("  ");
  Serial.print("Voltage = ");
  Serial.print(voltage);
  Serial.println(" V");

  // Display values on OLED 
  if( prevvoltage!=voltage)
  {
    deleteVoltage();
    printVoltage();
  }

  if( prevaveSensorValue!=aveSensorValue)
  {
    deleteSensorValue();
    printSensorValue();
  }
 




}
/*************************************************************************************************************************************************************/
void displaycalibration() {
   //display "Calibrating"
   tft.fillScreen(ST77XX_BLACK);
   tft.setTextColor(ST77XX_WHITE);
   tft.setTextSize(3);
   tft.setCursor(10,10);
   tft.println(F("+++++++++++"));
   tft.println(F(" START-UP"));
   tft.println(F("CALIBRATION"));
   tft.println(F("+++++++++++"));
   delay(500);
   tft.fillScreen(ST77XX_BLACK);
}

void printSensorValue()
{
  tft.setCursor(60, 140);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_BLUE);
  tft.println(aveSensorValue);
}

void deleteSensorValue()
{
  tft.setCursor(60, 140);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevaveSensorValue);
}

void printVoltage()
{
  tft.setCursor(40, 60);
  tft.setTextSize(6);
  tft.setTextColor(ST77XX_RED);
  tft.println(voltage,1);
}

void deleteVoltage()
{
  tft.setCursor(40, 60);
  tft.setTextSize(6);
  tft.setTextColor(ST77XX_BLACK);
  tft.println(prevvoltage,1);
}

void printLayout()
{
  tft.setCursor(10, 5);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("Voltage");
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

