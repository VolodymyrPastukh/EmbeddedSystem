#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "FS.h"
#include "SPIFFS.h"


/*----------------BLE Args----------------------*/
int scanTime = 5; 
BLEScan* pBLEScan;

/*----------------File Args----------------------*/
File bleDevicesFile;
const char* filePath = "/bleDevices.txt";

/*----------------Timer Args----------------------*/
int prescaler = 80;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

/*----------------WiFi login, password----------------------*/
const char* ssid       = "maksok";
const char* password   = "maksoklox";

/*----------------NTP Client----------------------*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const long  gmtOffset_sec = 7200;

/*----------------common args----------------------*/
String formattedTime;
bool doScan = false;
int timeStep = 10;
int currentTime = 0;




/*---------------- ( Timer method ) ---------------------*/
void IRAM_ATTR startBleScan() {
  portENTER_CRITICAL_ISR(&timerMux);
  doScan = true;
  portEXIT_CRITICAL_ISR(&timerMux);

  currentTime += timeStep;
}



/*---------------- < Callback class > ----------------------*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice device) {
      timeClient.forceUpdate();
      formattedTime = timeClient.getFormattedTime();
      
      // Name and TX
      bleDevicesFile = SPIFFS.open(filePath, "a");
      Serial.printf(" %s| Device: %s / TX: %d /",formattedTime, device.getName().c_str(), device.getTXPower());
      bleDevicesFile.printf(" %s| Device: %s / TX: %d /",formattedTime, device.getName().c_str(), device.getTXPower());

      // UUID
      if (device.haveServiceUUID()) {
      Serial.printf(" UUID: %s ", device.getServiceUUID().toString().c_str());
      bleDevicesFile.printf(" UUID: %s ", device.getServiceUUID().toString().c_str());
      }
      bleDevicesFile.println("|");
      Serial.println();

      bleDevicesFile.close();
    }
};



void setup() {
  Serial.begin(9600);

  //Check mounting SPIFFS
  if(!SPIFFS.begin(true)){
  Serial.println("Mounting SPIFFS error!");
  return;
  }

  //Connect to access point
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED to wifi");
  
  //WiFi connection
  Serial.println("");
  Serial.printf("ESP32 is connected to %s \n", ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //time
  timeClient.begin();
  timeClient.setTimeOffset(gmtOffset_sec);
  timeClient.forceUpdate();
  formattedTime = timeClient.getFormattedTime();
  Serial.printf("Start time : %s \n", formattedTime);

  //Timer
  timer = timerBegin(0, prescaler, true);
  timerAttachInterrupt(timer, &startBleScan, true);
  timerAlarmWrite(timer, timeStep * 1000000, true);
  timerAlarmEnable(timer);

  //Start scanning
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}





void loop() {
  if(doScan){
    if(currentTime <= timeStep * 3){
      
      //looking for devices
      BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
      Serial.print("Devices found: ");
      Serial.println(foundDevices.getCount());
      Serial.println("Scan done!\n");
      pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory


    }else if(currentTime == timeStep *4){
      //bleDevices.txt
      bleDevicesFile = SPIFFS.open(filePath, "r");
      Serial.print("bleDevicesFile.txt include: \"");
      while(bleDevicesFile.available()) {
        Serial.write(bleDevicesFile.read());
      }
      Serial.println("\"");   
      Serial.print("bleDevicesFile.txt size: ");
      Serial.println(bleDevicesFile.size());

      bleDevicesFile.close();
    }

  portENTER_CRITICAL(&timerMux);
  doScan = false;
  portEXIT_CRITICAL(&timerMux);
  }
}