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
  #define TFT_SDA       4     
  #define TFT_SCL       5
  #define TFT_CS        7
  #define TFT_RST       2     // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        6
  #define TFT_MOSI      10    // Data out
  #define TFT_SCLK      8     // Clock out
  #define ADCFACT       1024  

#elif defined(ARDUINO_ESP32_PICO)
  #define TFT_SDA       21     
  #define TFT_SCL       22
  #define TFT_MOSI      23    // Data out
  #define TFT_SCLK      18    // Clock out  #define 
  #define TFT_RST       5     // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        10
  #define TFT_CS        9
  #define ADCFACT       4095 

#elif defined(ARDUINO_AVR_NANO)
  #define TFT_SDA       23     
  #define TFT_SCL       24
  #define TFT_MOSI      11    // Data out
  #define TFT_SCLK      13    // Clock out  #define 
  #define TFT_RST       -1    // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        9
  #define TFT_CS        10
  #define ADCFACT       4095 

#elif defined(ARDUINO_TINYS3)
  #define TFT_SDA       8    
  #define TFT_SCL       9
  #define TFT_MOSI      35    // Data out
  #define TFT_SCLK      36    // Clock out  #define 
  #define TFT_RST       -1    // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        3
  #define TFT_CS        34
  #define TFT_BLK       2
  #define ADCFACT       4095 

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST       9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        8
#endif