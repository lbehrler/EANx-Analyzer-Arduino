# EANx-Analyzer

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
+ ESP32 or similar processor chip, with appropriate power supply
+ Adafruit ADS1115 or equivelent 12 or 16 bit 
+ I2C or SPI Liquid-crystal display (LCD) ST1306 or u8g2 
+ O2 Sensor
+ Project case

### The more advanced project with a OLED color display:
+ ESP32 or similar processor chip, with appropriate power supply
+ Adafruit ADS1115 or equivelent 12 or 16 bit 
+ I2C or SPI Organic light-emitting diode (OLED) ST7789
+ O2 Sensor
+ Project case

## Project Files

The project folder contains several files and one folder:

+ `LICENSE` - The license file for this project
+ `readme.md` - This file.
+ `EANx_Analizer.ino` - the basic application.

## Hardware Assembly
Future text 

## Installation
Installing the scripts onto your project chipset can be done using Arduino IDE 2.0

### Arduino IDE Requirements
- Windows - Win 10 and newer, 64 bits
- Linux - 64 bits
- Mac OS X - Version 10.14: "Mojave" or newer, 64 bits

The [Arduino IDE 2.0 tutorial](https://docs.arduino.cc/software/ide-v2/tutorials/getting-started/ide-v2-downloading-and-installing)

Alternately you can use esptools.py, Platform I/O or any number of script / binary installation managers. 

## Configuration
Future text 


## Revision History
+ 11/29/2022 - initial release

## Schematics 

*Seeeduino Xiao version with SPI OLED:*
![](https://github.com/lbehrler/EANx-Analyzer/blob/645330fc3275fe3a1c8c88061cc2e68e7b1bfda9/Seeed_Xiao_EANx_Analyzer_SPI_OLED%20schematic.png)

*Arduino Nano version with SPI OLED:*
![](https://github.com/lbehrler/EANx-Analyzer/blob/645330fc3275fe3a1c8c88061cc2e68e7b1bfda9/Nano_EANx_Analyzer_SPI_OLED%20schematic.png)
