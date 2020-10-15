#include <Arduino.h>

const int pin = 14;


void setup() {
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  int LRD_val = analogRead(pin);
  if(LRD_val != 0){
    Serial.println(LRD_val);
  }
  delay(1000);

}