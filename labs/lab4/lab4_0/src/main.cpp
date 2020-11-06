#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include "FS.h"
#include "SPIFFS.h"

#include <iostream>
#include <sstream>

//oneWire args----------------------------
const int bus = 4;     
OneWire oneWire(bus);
DallasTemperature sensors(&oneWire);

//file`s args----------------------------
File temperatureFile;
const char* filePath = "/temperature.txt";

//common args----------------------------
bool state = false;
int timeStep = 4;
int currentTime = 0;

//timer args------------------------------
int prescaler = 80;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR checkTemperature() {
  portENTER_CRITICAL_ISR(&timerMux);
  state = true;
  portEXIT_CRITICAL_ISR(&timerMux);

  currentTime += timeStep;
}

void setup() {
  Serial.begin(9600);
  sensors.begin();

  if(!SPIFFS.begin(true)){
  Serial.println("Mounting SPIFFS error!");
  return;
  }

  timer = timerBegin(0, prescaler, true);
  timerAttachInterrupt(timer, &checkTemperature, true);
  timerAlarmWrite(timer, timeStep * 1000000, true);
  timerAlarmEnable(timer);
}

void loop() {
  if(state){
    if(currentTime <= 60){
      sensors.requestTemperatures(); 
      float temperatureC = sensors.getTempCByIndex(0);
      // std::stringstream ss;
      // std::string tempC;
      // ss << currentTime << "s: [" << temperatureC << "ºC]";
      // ss >> tempC;
      // const char *result = tempC.c_str();

      temperatureFile = SPIFFS.open(filePath, FILE_APPEND);
      if(temperatureFile.print(currentTime) && temperatureFile.print("s: [") && temperatureFile.print(temperatureC) && temperatureFile.println(" ºC]")){
        Serial.print(currentTime);
        Serial.println("sec success...");
      }else{
        Serial.println("Appending failed(");
      }
      temperatureFile.close();

    }else if(currentTime == 64){
      temperatureFile = SPIFFS.open(filePath, FILE_READ);
      Serial.print("temperature.txt include: \"");
      while(temperatureFile.available()) {
        Serial.write(temperatureFile.read());
      }
      Serial.println("\"");   

      Serial.print("temperature.txt size: ");
      Serial.println(temperatureFile.size());

      temperatureFile.close();
    }

  portENTER_CRITICAL(&timerMux);
  state = false;
  portEXIT_CRITICAL(&timerMux);
  }
}