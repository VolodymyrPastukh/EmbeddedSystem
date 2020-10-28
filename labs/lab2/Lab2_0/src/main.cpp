#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

const int bus = 4;     
OneWire oneWire(bus);

DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  delay(5000);
}