#include <Arduino.h>

#define DAC2 26 

void setup() {
}

void loop() {
  for (int i = 0; i < 256; i++) { 
    dacWrite(DAC2, i);
    delay(100);
  }
  
  for (int i = 255; i > -1; i--) {
    dacWrite(DAC2, i);
    delay(100);
  }
}