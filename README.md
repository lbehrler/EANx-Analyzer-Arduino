# EANx-Analyzer
  EANx Analysis with output to an OLED color display

  Reads an analog input on pin, converts it to voltage, grabs a running average of ADC values and and prints the result to the display and debug to Serial Monitor.

  Based on prior EANx scripts: 
  - https://github.com/ppppaoppp/DIY-Nitrox-Analyzer-04_12_2019.git
  - https://github.com/ejlabs/arduino-nitrox-analyzer.git
 
 The EANx Analyzer project is meant to be a DIY O2 Analyzer.  
 
 Use at your own risk.  If you are using the device and software for critical purposes, please seek proper training and know the limits of devices of this type. 

The code can be customized to match a wide variety of hardware.  The current version in the main branch is setup to use an ESP32-Pico-D4, a Seeeduino Xiao, or an Arduino Nano with an SPI interface OLED screen 240 x 240.

*Schematic Arduino Nano version:*
![](https://github.com/lbehrler/EANx-Analyzer/blob/eef317a30d3a1932c77d38c698fbaf5213d5280e/EANx_Analyzer_SPI_OLED%20schematic.png)
