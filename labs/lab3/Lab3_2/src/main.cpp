#include <Arduino.h>

const int pin = 14;

volatile bool state = false;
int counter = 0;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  state = true;
  portEXIT_CRITICAL_ISR(&timerMux);

  counter += 4;
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 4000000, true);
  timerAlarmEnable(timer);
}

void loop() {
  if(state){
    int LRD_val = analogRead(pin);
    Serial.print(counter);
    Serial.print("s: Light = ");
    Serial.println(LRD_val);

    portENTER_CRITICAL(&timerMux);
    state = false;
    portEXIT_CRITICAL(&timerMux);
  }

}