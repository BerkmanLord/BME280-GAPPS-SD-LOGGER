/*Temparature, Humidity & Pressure Logger - Berkman Lord Gülenç
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

*/


#include <Arduino.h>
#include <Wire.h>

//WIFI Libraries
#include <WiFi.h>

//https://github.com/jbuszkie/HTTPSRedirect
//For ESP32, make sure to uncomment Line 89, stop(); in HTTPSRedirect.h
#include <HTTPSRedirect.h>


//OLED Display Libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Bosch BME280 Sensor Libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//SD Card Reader Libraries
#include <SPI.h>
#include <mySD.h>

//Hard-coded Wifi Settings - Temporary
const char* ssid     = "SSID1";
const char* password = "PASS1";

const char* ssid2     = "SSID2";
const char* password2 = "PASS2";

bool wifiStat = 0; //0 - Not Connected, 1 - Connected, 2 - No IP
bool uploadStat = 0;
int sdStat = 1; //0 - Not Initialized, 1 - Initialized, 3 - No File
bool bme280Stat = 1;  //0 - Not Initialized, 1 - Initialized
bool oledStat = 1; //0 - Not Initialized, 1 - Initialized

bool booted = 0;
int uploadCounter = 1;

//Logging sensor data to Serial Console. Disabled to avoid clutter.
//Enable by typing 1 to Serial Monitor as Binary.
bool serialLoggingEnabled = false;

//User Control Pins - Digital - Internal Pullup
//int debugPin     = 15;
//int measurePin   = 2;

        //OLED Display Initialization
        #define SCREEN_WIDTH 128 // OLED display width, in pixels
        #define SCREEN_HEIGHT 64 // OLED display height, in pixels
        #define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
        Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

        //Screen Icons - Icons are made using the LCD Image Converter and Pixilart.com
		//https://www.pixilart.com
		//https://sourceforge.net/projects/lcd-image-converter/
		//https://create.arduino.cc/projecthub/Arnov_Sharma_makes/displaying-your-own-photo-on-oled-display-5a8e8b
        static const uint8_t irhmIcon[240] = {
    0x00, 0x7f, 0xff, 0xff, 0xff, 0xe0, 
    0x00, 0xa0, 0x00, 0x00, 0x00, 0x60, 
    0x01, 0xa0, 0x00, 0x00, 0x00, 0xa0, 
    0x03, 0x20, 0x00, 0x00, 0x01, 0x20, 
    0x06, 0x20, 0x00, 0x00, 0x02, 0x20, 
    0x0c, 0x20, 0x00, 0x00, 0x04, 0x20, 
    0x18, 0x20, 0x00, 0x00, 0x08, 0x20, 
    0x30, 0x20, 0x00, 0x00, 0x10, 0x20, 
    0x60, 0x20, 0x00, 0x00, 0x20, 0x20, 
    0xc0, 0x20, 0x00, 0x00, 0x40, 0x20, 
    0xff, 0xff, 0xff, 0xff, 0x80, 0x20, 
    0x90, 0x20, 0x00, 0x00, 0x80, 0x20, 
    0x98, 0x20, 0x00, 0x00, 0x80, 0x20, 
    0x9e, 0x20, 0x00, 0x00, 0x8e, 0x20, 
    0x9f, 0x20, 0x00, 0x00, 0xfe, 0x20, 
    0x9f, 0xff, 0xff, 0xff, 0xfe, 0x20, 
    0x8f, 0xff, 0xff, 0xff, 0xfe, 0x20, 
    0x87, 0xff, 0xff, 0xff, 0xfc, 0x20, 
    0x87, 0xff, 0xff, 0xff, 0xfc, 0x20, 
    0x87, 0xfe, 0x00, 0x1f, 0xfc, 0x20, 
    0x8f, 0xff, 0x00, 0x3f, 0xfe, 0x20, 
    0x8f, 0xbf, 0x80, 0x7e, 0xbe, 0x20, 
    0x8f, 0x3f, 0x80, 0xff, 0x9e, 0x20, 
    0x9f, 0x3f, 0xc1, 0xff, 0x9e, 0x20, 
    0x9f, 0x3f, 0xe3, 0xff, 0x9e, 0x20, 
    0x9f, 0x3f, 0xf7, 0xff, 0x9e, 0x20, 
    0x9f, 0x3f, 0xff, 0xef, 0xbe, 0x20, 
    0x9e, 0x3c, 0xff, 0xc4, 0xbc, 0x20, 
    0x9f, 0x08, 0xff, 0xe0, 0xfc, 0x20, 
    0x8f, 0x83, 0xff, 0xf9, 0xff, 0xe0, 
    0x87, 0xcf, 0xff, 0xff, 0xf8, 0x20, 
    0x87, 0xff, 0xfe, 0xff, 0xf0, 0x60, 
    0x83, 0xff, 0x9e, 0x7f, 0xe0, 0xc0, 
    0x83, 0xff, 0x1c, 0x1f, 0xc1, 0x80, 
    0x86, 0xfc, 0x08, 0x06, 0x83, 0x00, 
    0x8c, 0x00, 0x00, 0x00, 0x86, 0x00, 
    0x98, 0x00, 0x00, 0x00, 0x8c, 0x00, 
    0xb0, 0x00, 0x00, 0x00, 0x98, 0x00, 
    0xe0, 0x00, 0x00, 0x00, 0xf0, 0x00, 
    0xff, 0xff, 0xff, 0xff, 0xe0, 0x00
        };
        static const uint8_t tickIcon[24] = {
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0xc0, 
    0x21, 0xe0, 
    0x73, 0xc0, 
    0x7f, 0x80, 
    0x3f, 0x00, 
    0x1e, 0x00, 
    0x0c, 0x00, 
    0x04, 0x00, 
    0x00, 0x00
};
        static const uint8_t crossIcon[24] = {
    0x00, 0x00, 
    0x20, 0x40, 
    0x70, 0xe0, 
    0x79, 0xc0, 
    0x3f, 0x80, 
    0x1e, 0x00, 
    0x0f, 0x00, 
    0x1f, 0x80, 
    0x33, 0x80, 
    0x61, 0xc0, 
    0x40, 0x80, 
    0x00, 0x00
};       
        static const uint8_t wifiIcon[24] = {
    0x00, 0x00, 
    0x3f, 0xc0, 
    0x40, 0x20, 
    0x80, 0x10, 
    0x1f, 0x80, 
    0x20, 0x40, 
    0x40, 0x20, 
    0x0f, 0x00, 
    0x10, 0x80, 
    0x26, 0x40, 
    0x06, 0x00, 
    0x0f, 0x00
};
        static const uint8_t sdIcon[24] = {
    0x00, 0x00, 
    0x7f, 0xc0, 
    0x55, 0x40, 
    0x55, 0x40, 
    0x7f, 0xc0, 
    0x7f, 0xe0, 
    0x7f, 0xe0, 
    0x7f, 0xc0, 
    0x7f, 0xc0, 
    0x7f, 0xe0, 
    0x40, 0x60, 
    0x7f, 0xe0
};
        static const uint8_t uploadIcon[24] = {
    0x00, 0x00, 
    0x7e, 0x00, 
    0xc3, 0xf0, 
    0x80, 0x10, 
    0x82, 0x10, 
    0x87, 0x10, 
    0x8a, 0x90, 
    0x82, 0x10, 
    0x82, 0x10, 
    0x80, 0x10, 
    0xff, 0xf0, 
    0x00, 0x00
};

        //Bosch BME280 Initialization
        #define SEALEVELPRESSURE_HPA (1013.25)
        Adafruit_BME280 bme;

        //SD Card Initialization
        const int chipSelect = 5;
        int logCounter =1;


//Timer Variables for Measurement and Logging - 60000UL - 60 Seconds
unsigned long readSensorTimer = 5010UL;
const unsigned long readSensorInterval = 5000UL;

unsigned long displayStatsTimer = 5010UL;
const unsigned long displayStatsInterval = 5000UL;

unsigned long logToFileTimer = 60010UL;
const unsigned long logToFileInterval = 60000UL; 

unsigned long uploadStatsTimer = 0;
const unsigned long uploadStatsInterval = 60000UL;


void setup() {
  // put your setup code here, to run once:

            //Serial Setup
            Serial.begin(115200);
            Serial.println("-- IRHM Air Quality Logger --");
  
  //User Control Pins
  //pinMode(debugPin, INPUT_PULLUP);
  //pinMode(measurePin, INPUT_PULLUP);

        // OLED Display Setup
        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));    
        oledStat = 0;
        }

        //BME280 Sensor Setup
        if (! bme.begin(0x76, &Wire)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        bme280Stat = 0;
        }
               bme.setSampling(Adafruit_BME280::MODE_FORCED,
                       Adafruit_BME280::SAMPLING_X1, // temperature
                       Adafruit_BME280::SAMPLING_X1, // pressure
                       Adafruit_BME280::SAMPLING_X1, // humidity
                       Adafruit_BME280::FILTER_OFF   );
                       // suggested rate is 1/60Hz (1m)
                       //delayTime = 30000; // in milliseconds
        
        //SD Card Setup
        if (!SD.begin(chipSelect)) {
          Serial.println("Card failed, or not present");
        sdStat = 0;
        }



       Serial.println("---------------");
       Serial.println("- Active Settings -");
       Serial.println("Filter Off | Manual Mode | 1X Oversampling Measurement");
       Serial.println("1 - Enable Serial Logging | 0 - Disable Serial Logging");
       Serial.println("---------------");
       Serial.println("- On-board  Devices  Status- ");
       Serial.print("Wireless LAN = ");Serial.print(wifiStat);Serial.print(" SD Card = ");Serial.print(sdStat);Serial.print(" BME280 = ");Serial.print(bme280Stat);Serial.print(" OLED Display = ");Serial.println(oledStat);
       Serial.println("---------------");

    readSensorTimer = millis();
    displayStatsTimer = millis();
    logToFileTimer = millis();
    uploadStatsTimer = millis();
}

int wifiConnected(){

      //Try Primary Network
      if (WiFi.status() != WL_CONNECTED) {
             wifiStat = 0;
             uploadStat = 0;
             WiFi.begin(ssid, password);
             WiFi.setHostname("IRHM Logger. 1");
             Serial.println("\n");
             Serial.print("Connecting to ");
             Serial.print(ssid); Serial.print(".");
            for (int i = 0; i < 5; i++)
            {
              Serial.print(".");
              delay (1000);
            }
            }

      //Try Secondary Network
      if (WiFi.status() != WL_CONNECTED) {
             wifiStat = 0;
             uploadStat = 0;
             WiFi.begin(ssid2, password2);
             WiFi.setHostname("IRHM Logger. 1");
             Serial.println("\n");
             Serial.print("Connecting to ");
             Serial.print(ssid2); Serial.print(".");
            for (int i = 0; i < 5; i++)
            {
              Serial.print(".");
              delay (1000);
            }
            }

            if(WiFi.status() != WL_CONNECTED) {
              Serial.println("\n");
              Serial.println("Unable to connect to WIFI!");
              wifiStat = 0;
              return 0;
            } else {
            Serial.println("\n");
            Serial.println("Wifi Connection established!");  
            Serial.print("IP address:");
            Serial.println(WiFi.localIP());
            wifiStat = 1;
            return 1;
            }
     
}

void uploadStats(float temp, float hum, float psr) {
       
  // This function uploads data to Google Sheets.
  // More info and original script can be found at https://github.com/StorageB/Google-Sheets-Logging 
  
          if (wifiConnected())
          {      
          Serial.println("Beginning Cloud Upload...");
          //Cloud Connection Settings
          //Enter Google Script Deployment ID:
                    Serial.print("Heap Size 1 = "); Serial.println(esp_get_free_heap_size());
          const char *GScriptId = "ENTERYOURGOOGLESCRIPTID";
          // Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
          String payload_base =  "{\"command\": \"append_row\", \"sheet_name\": \"Status Reports\", \"values\": ";
          String payload = "";
          // Google Sheets setup (do not edit)
          const char* host = "script.google.com";
          const int httpsPort = 443;
          const char* fingerprint = "";
          String url = String("/macros/s/") + GScriptId + "/exec?cal";
          HTTPSRedirect* client = nullptr;
                   

      // Use HTTPSRedirect class to create a new TLS connection
      client = new HTTPSRedirect(httpsPort);
      client->setInsecure();
      client->setPrintResponseBody(true);
      client->setContentTypeHeader("application/json");
  
          Serial.print("Connecting to ");
          Serial.println(host);

          // Try to connect for a maximum of 5 times
          bool flag = false;
          for (int i=0; i<5; i++){ 
            int retval = client->connect(host, httpsPort);
            if (retval == 1){
                flag = true;
                Serial.println("Connected");
            break;
            }
            else
              Serial.println("Connection failed. Retrying...");
            }

            if (!flag){
              Serial.print("Could not connect to server: ");
              Serial.println(host);
            }
  
            delete client;    // delete HTTPSRedirect object
            client = nullptr; // delete HTTPSRedirect object

          flag = false;
          if (!flag)
          {
              client = new HTTPSRedirect(httpsPort);
              client->setInsecure();
              flag = true;
              client->setPrintResponseBody(true);
              client->setContentTypeHeader("application/json");
          }
                     Serial.print("Heap Size 2 = "); Serial.println(esp_get_free_heap_size());
          if (client != nullptr)
          {
            if (!client->connected())
            {
                client->connect(host, httpsPort);
            }
          }else{
          Serial.println("Error creating client object!");
          }

        // Create json object string to send to Google Sheets. Pascals are converted to mmHg
        payload = payload_base + "\"" + uploadCounter++ + "," + temp + "," + hum + "," + psr/133.322 + "\"}";

        // Publish data to Google Sheets
        Serial.println("Publishing data...");
        Serial.println(payload);
        if(client->POST(url, host, payload)){ 
          // do stuff here if publish was successful
          uploadStat = 1;
        }
          else{
         // do stuff here if publish was not successful
        Serial.println("Error while connecting");
          uploadStat = 0;
        }
  
  
  Serial.println("Data uploaded to Cloud.");
            delete client;    // delete HTTPSRedirect object
            client = nullptr; // delete HTTPSRedirect object
      }

}

void displayStats(float temp, float hum, float psr, int wifiStat) {
  // This function updates the display with real-time sensor data.

    //Serial Monitor
    if (serialLoggingEnabled)
    {
    Serial.println("-- Sensor Data Acquired ----------");
    Serial.print("Temparature - ");Serial.print(temp); Serial.println(" *C");
    Serial.print("Humidity    - ");Serial.print(hum); Serial.println("%");
    Serial.print("Pressure    - ");Serial.print(psr/133.322); Serial.println(" mmHg");
    Serial.print("Apx. Alti.  - ");Serial.print(temp); Serial.println(" Meters");
    }


          //OLED Display
          display.clearDisplay();
          display.setTextSize(1);             // Normal 1:1 pixel scale
          display.setTextColor(WHITE);        // Draw white text
          display.setCursor(50,0);
          display.print(temp);
          display.print(" C"));
          display.setCursor(50,11);
          display.print(hum);
          display.print(" %"));
          display.setCursor(50,22);
          display.print(psr/133.322);
          display.print(" mmHg");
          display.drawBitmap(0, 0, irhmIcon, 43, 40, 1);
          display.drawBitmap(0, 41, wifiIcon, 12, 12, 1);
          if (wifiStat == 1){display.drawBitmap(0, 54, tickIcon, 12, 12, 1);}else{display.drawBitmap(0, 54, crossIcon, 12, 12, 1);}
          display.drawBitmap(13, 41, uploadIcon, 12, 12, 1);
          if (uploadStat == 1){display.drawBitmap(13, 54, tickIcon, 12, 12, 1);}else{display.drawBitmap(13, 54, crossIcon, 12, 12, 1);}
          display.drawBitmap(26, 41, sdIcon, 12, 12, 1);
          if (sdStat == 1){display.drawBitmap(26, 54, tickIcon, 12, 12, 1);}else{display.drawBitmap(26, 54, crossIcon, 12, 12, 1);}

              //Wifi Status
              if (wifiStat == 1)
              {
              display.setCursor(50,33);
              display.print(WiFi.localIP());
              }else if (wifiStat == 0){
              display.setCursor(50,33);
              display.print("No network :(");
              }else{
              display.setCursor(50,33);
              display.print("Upload Failed!"); 
              }
         display.display();

        ////User Control Pins
        /*if (digitalRead(debugPin) == LOW)
            {
                //Debug Info
            }*/

  Serial.println("Display updated successfully.");
}

void logToFile(float temp, float hum, float psr){

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(logCounter++);
    dataFile.print(",");
    dataFile.print(temp);
    dataFile.print(",");
    dataFile.print(hum);
    dataFile.print(",");
    dataFile.println(psr/133.322);

    dataFile.close();
    // print to the serial port too:
    Serial.println("Data logged.");
    sdStat = 1;
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("Error opening datalog.csv");
    sdStat = 2;
  }
}

void readSensor() {
  bme.takeForcedMeasurement();
  Serial.println("Sensor data acquired.");
}


void loop() {
  // put your main code here, to run repeatedly:

  if (esp_get_free_heap_size() <= 15000)
  {
  Serial.print("Heap Size Below Treshold = "); Serial.println(esp_get_free_heap_size());
  }
  
          
 //Sensor is read every 30 seconds.
 //Display is updated by Sensor Read function.
 //Data is uploaded every minute.

  if (booted == false){
    Serial.println("System Booting...");
    Serial.print("Heap Size Initial = "); Serial.println(esp_get_free_heap_size());
          display.clearDisplay();
          display.setTextSize(1);             // Normal 1:1 pixel scale
          display.setTextColor(WHITE);        // Draw white text
          display.setCursor(50,0);
          display.print("Atmo. Logger");
          display.setCursor(50,11);
          display.print("Initialising");
          display.setCursor(50,20);
          display.print("------------");
          display.drawBitmap(0, 0, irhmIcon, 43, 40, 1);
          display.drawBitmap(0, 41, wifiIcon, 12, 12, 1);
          if (wifiConnected() == 1){display.drawBitmap(0, 54, tickIcon, 12, 12, 1);}else{display.drawBitmap(0, 54, crossIcon, 12, 12, 1);}
          display.drawBitmap(13, 41, uploadIcon, 12, 12, 1);
          if (uploadStat == 1){display.drawBitmap(13, 54, tickIcon, 12, 12, 1);}else{display.drawBitmap(13, 54, crossIcon, 12, 12, 1);}
          display.drawBitmap(26, 41, sdIcon, 12, 12, 1);
          if (sdStat == 1){display.drawBitmap(26, 54, tickIcon, 12, 12, 1);}else{display.drawBitmap(26, 54, crossIcon, 12, 12, 1);}

          display.display();
  
        // System Check
         if (((wifiStat == 0 || wifiStat == 2) && (sdStat == 0 || sdStat == 2)) || bme280Stat == 0)
         {
           Serial.println("Unable to Log Data. Components are either missing or inoperable.");
           Serial.print("Wireless LAN = ");Serial.print(wifiStat);Serial.print(" SD Card = ");Serial.print(sdStat);;Serial.print(" BME280 = ");Serial.println(bme280Stat);
           Serial.println("Restarting in 5 seconds!");
           display.setCursor(50,30);
           display.print("SD/WiFi/Sens");
           display.setCursor(61,41);
           display.print("Missing");
           display.setCursor(53,51);
           display.print("Restarting!");
           display.display();
           delay(20000);
           ESP.restart();
         }

         uploadStats(bme.readTemperature(), bme.readHumidity(), bme.readPressure());

    booted = true;
    delay(2000);
  }

    if (Serial.available() > 0) {
    char command = Serial.read(); // read the next char received
    if (command == 1) {
      serialLoggingEnabled = true;
      }
      else if (command == 0)
      {
      serialLoggingEnabled = false;
      }
      
}
  //Read Sensor
  if(((millis() -  readSensorTimer) >= readSensorInterval) && bme280Stat == 1){
    readSensor();
    readSensorTimer = millis();
  }

  //Display Stats on OLED
  if(((millis() -  displayStatsTimer) >= displayStatsInterval) && bme280Stat == 1 && oledStat == 1){
    displayStats(bme.readTemperature(), bme.readHumidity(), bme.readPressure(), wifiStat);
    displayStatsTimer = millis();
  }

  //Log sensor data to the SD Card
  if(((millis() - logToFileTimer) >= logToFileInterval) && bme280Stat == 1 && sdStat == 1){
    logToFile(bme.readTemperature(), bme.readHumidity(), bme.readPressure());
    logToFileTimer = millis();
  }

  //Upload sensor data to Google Sheets
  if(((millis() - uploadStatsTimer) >= uploadStatsInterval) && bme280Stat == 1){
    uploadStats(bme.readTemperature(), bme.readHumidity(), bme.readPressure());
    uploadStatsTimer = millis();
  }

              /*if (digitalRead(measurePin) == LOW){
              readSensor();
              delay(5000);
              }

              if (digitalRead(debugPin) == LOW){
              Serial.print("Sensor Timer - ");
              Serial.println(readSensorTimer);
              Serial.print("Upload Timer - ");
              Serial.println(uploadStatsTimer);
              Serial.print("Millis - ");
              Serial.println(millis());
                readSensor();
                delay(5000);
              }*/
}