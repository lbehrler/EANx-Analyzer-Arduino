# EANx-Analyzer-Arduino / ESP32 

> **Note:**: If you have an issue with this project, please review your hardware docuemntation first.  If you are still having an issue or have a software specific issue, please open an issue [here](https://github.com/lbehrler/EANx-Analyzer/issues).

EANx Analysis with output to an OLED color display.
The code can be customized to match a wide variety of hardware.  The current version in the main branch is setup to use an ESP32-Pico-D4, a Seeeduino Xiao, or an Arduino Nano with an SPI interface OLED screen 240 x 240.

In its simplest form the EANx Analyzer reads an analog input from an ADS1115, converts it to voltage, grabs a running average of ADC values and and prints the result to the display and debug to Serial Monitor.

The EANx Analyzer project is meant to be a DIY O2 Analyzer.  
 
**Use at your own risk.**  If you are using the device and software for critical purposes, please seek proper training and know the limits of devices of this type. 

## Acknowledgements
Some of this code is adapted from other EANx project scripts: 
  - https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019.git
  - https://github.com/ejlabs/arduino-nitrox-analyzer.git

## Required Components

### The Basic [monochrome] project:
+ Arduino, ESP32 or similar processor chip, with appropriate power supply
+ Adafruit ADS1115 or equivelent 16 bit 
+ I2C or SPI Organic light-emitting diode (OLED) ST1306 or u8g2 
+ O2 Sensor
+ Project case

### The more advanced project with a OLED color display:
+ Arduino, ESP32 or similar processor chip, with appropriate power supply
+ Adafruit ADS1115 or equivelent 16 bit 
+ I2C or SPI Organic light-emitting diode (OLED) ST7789
+ O2 Sensor
+ Project case

## Project Files

The project folder contains several files and one folder:

+ `LICENSE` - The license file for this project
+ `readme.md` - This file.
+ `EANx_Analyzer_XXXXXXX.ino` - the basic application.
+ `pin_config.h` - Header file for various ESP32 / Arduino pinout settings. 
+ `OTA.h` - Simple OTA upate header (optional utility).
+ `bat_stat.h` - Simple Battery status script for ESP32 based boards.

## Installation
Installing the scripts onto your project chipset can be done using Arduino IDE 2.0

### Arduino IDE Requirements
- Windows - Win 10 and newer, 64 bits
- Linux - 64 bits
- Mac OS X - Version 10.14: "Mojave" or newer, 64 bits

The [Arduino IDE 2.0 tutorial](https://docs.arduino.cc/software/ide-v2/tutorials/getting-started/ide-v2-downloading-and-installing)

Alternately you can use esptools.py, Platform I/O or any number of script / binary installation managers. 

## Configuration
Depending on your hardware configuration and components used you will have to set your pin definitions in the header files. 

If you are using a basic ST3%XX or ST13XX series display you can use [U8g2] (https://github.com/olikraus/u8g2) or the Adafruit librarires. For more advance display features and better efficiency the [Bodmer TFT eSPI library] (https://github.com/Bodmer/TFT_eSPI) can be configured allowing for greater options. If you are using the EANx Analyzer OLED TFT eSPI.ino then the TFT eSPI library is required. 

## Revision History
+ 11/29/2022 - initial release
+ 12/30/2022 - prototype release

## Schematics 

*Seeeduino Xiao version with SPI OLED:*
![](https://github.com/lbehrler/EANx-Analyzer/blob/645330fc3275fe3a1c8c88061cc2e68e7b1bfda9/Seeed_Xiao_EANx_Analyzer_SPI_OLED%20schematic.png)

*Arduino Nano version with SPI OLED:*
![](https://github.com/lbehrler/EANx-Analyzer-Arduino/blob/main/Nano_EANx_Analyzer_SPI_OLED%20schematic.png)

## Prototype

The current prototype unit is built using:
+ LilyGo-T-OI-PLUS [ESP32 C3 with 16340 Battery holder](https://github.com/Xinyuan-LilyGO/LilyGo-T-OI-PLUS)
+ Generic ADS1115 
+ SPI Organic light-emitting diode (OLED) ST7789
+ O2 Sensor
+ 1/4 size Electro Cookie Proto Board PCB
+ Project case

*Side View Prototype with SPI OLED*
![](https://github.com/lbehrler/EANx-Analyzer-Arduino/blob/main/PrototypeSideView.jpg)

*Operating Prototype*
![](https://github.com/lbehrler/EANx-Analyzer-Arduino/blob/main/PrototypeAssembled.jpg)
