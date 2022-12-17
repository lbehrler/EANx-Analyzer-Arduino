// Chip Specific settings for SPI OLED 
#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#elif defined(SEEED_XIAO_M0)  // Seeed XIAO
  #define TFT_SDA       4     
  #define TFT_SCL       5
  #define TFT_CS        7
  #define TFT_RST       2     // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        6
  #define TFT_MOSI      10    // Data out
  #define TFT_SCLK      8     // Clock out
  #define BUTTON_PIN    1
  #define OTA           0
  #define OTADEVICE     SEEED_XIAO_EANx
  #define ADCFACT       1024  

#elif defined(ARDUINO_XIAO_ESP32C3)  // Seeed XIAO ESP32 C3
  #define TFT_SDA       4     
  #define TFT_SCL       5
  #define TFT_CS        7
  #define TFT_RST       2     // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC        6
  #define TFT_MOSI      10    // Data out
  #define TFT_SCLK      8     // Clock out
  #define BUTTON_PIN    1
  #define OTA           1
  #define OTADEVICE     "XIAO_ESP32_EANx"
  #define ADCFACT       1024  

#elif defined(ARDUINO_ESP32_PICO)
  #define TFT_SDA       21     
  #define TFT_SCL       22
  #define TFT_MOSI      23    // Data out
  #define TFT_SCLK      18    // Clock out  #define 
  #define TFT_RST       5     // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        10
  #define TFT_CS        9
  #define BUTTON_PIN    2
  #define OTA           1
  #define ADCFACT       4095 

#elif defined(ARDUINO_AVR_NANO)
  #define TFT_SDA       23     
  #define TFT_SCL       24
  #define TFT_MOSI      11    // Data out
  #define TFT_SCLK      13    // Clock out  #define 
  #define TFT_RST       -1    // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        9
  #define TFT_CS        10
  #define BUTTON_PIN    2
  #define OTA           0
  #define ADCFACT       4095 

#elif defined(ARDUINO_TINYS3)
  #define TFT_SDA       8    
  #define TFT_SCL       9
  #define TFT_MISO      37
  #define TFT_MOSI      35    // Data out
  #define TFT_SCLK      36    // Clock out  #define 
  #define TFT_RST       -1    // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        3
  #define TFT_CS        34
  #define TFT_BLK       2
  #define BUTTON_PIN    2
  #define OTA           1
  #define OTADEVICE     "TINYS3_EANx"
  #define ADCFACT       4095 

#elif defined(ARDUINO_TTGO)
  #define TFT_SDA       18    
  #define TFT_SCL       19
  #define TFT_MOSI      7    // Data out
  #define TFT_SCLK      5    // Clock out  #define 
  #define TFT_RST       -1    // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        4
  #define TFT_CS        10
  #define TFT_BLK       2
  #define BUTTON_PIN    2
  #define OTA           1
  #define OTADEVICE     "TTGO_TOI_EANx"
  #define ADCFACT       4095 

#elif defined(ARDUINO_ESP32S3)
  #define PIN_LCD_BL                   38

  #define PIN_LCD_D0                   39
  #define PIN_LCD_D1                   40
  #define PIN_LCD_D2                   41
  #define PIN_LCD_D3                   42
  #define PIN_LCD_D4                   45
  #define PIN_LCD_D5                   46
  #define PIN_LCD_D6                   47
  #define PIN_LCD_D7                   48

  #define PIN_POWER_ON                 15

  #define PIN_LCD_RES                  5
  #define PIN_LCD_CS                   6
  #define PIN_LCD_DC                   7
  #define PIN_LCD_WR                   8
  #define PIN_LCD_RD                   9

  #define PIN_BUTTON_1                 0
  #define PIN_BUTTON_2                 14
  #define PIN_BAT_VOLT                 4

  #define PIN_IIC_SCL                  17
  #define PIN_IIC_SDA                  18

  #define PIN_TOUCH_INT                16
  #define PIN_TOUCH_RES                21

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_RST       -1    // Or set to -1 and connect to Arduino RESET pin                                            
  #define TFT_DC        4
  #define TFT_CS        10
#endif