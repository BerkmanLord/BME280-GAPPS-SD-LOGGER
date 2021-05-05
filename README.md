# BME280-GAPPS-SD-LOGGER
Temperature, Humidity &amp; Pressure Logger to Google Sheets &amp; SD Card with OLED Readout

This script will log the data from BME280 sensor to Google Sheets and to an SD Card that is connected through SPI to ESP32.
In addition, it will display the stats on Adafruit_SSD1306 OLED display.

Please make sure to add all the libraries, HTTPSRedirect must be downloaded from the link provided.

https://github.com/jbuszkie/HTTPSRedirect

Click on the link below to learn more about the Google Sheets script by StorageB.

https://github.com/StorageB/Google-Sheets-Logging

- Pinout for ESP32-DevKitC V4
- Card Reader SPI
VIN  - 5V - PLEASE MAKE SURE YOUR CARD READER SUPPORTS 5VOLTS.
CS   - GPIO 5
MOSI - GPIO 23
CLK  - GPIO 18
MISO - GPIO 19
GND	 - GND

- I2C
SCL GPIO 22
SDA GPIO 21
