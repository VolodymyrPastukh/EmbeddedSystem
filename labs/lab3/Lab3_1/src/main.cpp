#include <Arduino.h>

const int BUTTON_PIN = 16;
int counter = 0;
bool pressed = false;

void IRAM_ATTR buttonPress(){
  counter += 1;
  pressed = true;
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(BUTTON_PIN, buttonPress, FALLING);
}

void loop() {
  if(pressed){
    Serial.printf("Button %u \n", counter);
    pressed = false;
  }

  if(millis() > 60000){
    detachInterrupt(BUTTON_PIN);
	  Serial.println("Bachok poti");
  }
}