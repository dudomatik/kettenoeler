#include <EEPROM.h>

#define pump 2
#define reed 3
#define rainSwitch 4
#define ledPump 5
#define ledFail 6
#define calibrateButton 8
#define pumpButton 10

#define firstPumpAction 15           //~800m (500)
#define pumpAction 5               //~8km (5000)
#define rainPumpAction 3           //~4km (2500)
#define pumpTime 1000
#define emergencyTime 15000          //15min (900000)
#define emergencyPumpTime 3000      //10min (600000)
#define emergencyPumpTimeRain 3000  //5min (300000)
#define reedDebouce 20

volatile int rotation = 0;
byte mode = 0;
unsigned long lastReedTime = 0;
int pumpDelay = 0;                    //emergency mode
unsigned long lastPumpTime = 0;       //emergency mode
volatile unsigned long lastRotTime = 0;
volatile boolean emergency = false;

void setup() {
   Serial.begin(9600);
   pinMode(pump, OUTPUT);
   pinMode(pumpButton, INPUT_PULLUP);
   pinMode(rainSwitch, INPUT_PULLUP);
   pinMode(ledPump, OUTPUT);
   pinMode(ledFail, OUTPUT);
   pinMode(13, OUTPUT);
   attachInterrupt(digitalPinToInterrupt(reed), countRotation, FALLING);
   digitalWrite(reed, HIGH);
}

void loop() {
  
  //manual pump button
  if (digitalRead(pumpButton)){
    digitalWrite(ledPump, LOW);
    digitalWrite(pump, LOW);
  }
  else {
    digitalWrite(ledPump, HIGH);
    digitalWrite(pump, HIGH);
  }
  
  //check rain mode switch (only if start mode 0 is over)
  if (mode != 0){
    if (digitalRead(rainSwitch)){
      mode = 2;
    }
    else {
      mode = 1;
    }
  }

  // compare rotations with set limits
  switch (mode) {
    case 0:
      // start mode
      if (rotation > firstPumpAction){
          rotation = 0;
          pumpCycle();
          mode = 2;
        }
      break;
    case 1:
      // rain mode
      if (rotation > rainPumpAction){
          rotation = 0;
          pumpCycle();
        }
      break;
    default: 
      // default mode
      if (rotation > pumpAction){
          rotation = 0;
          pumpCycle();
        }
    break;
  }
  
  //emergency mode
  if (millis() - lastReedTime > emergencyTime) {
    emergency = true;
    digitalWrite(ledFail, HIGH);
    do {
      if(millis() - lastPumpTime > pumpDelay){
        Serial.println("emergencyPUMP");
        
        Serial.println(pumpDelay);
        pumpCycle();
        lastPumpTime = millis();
      }
      if (digitalRead(rainSwitch)){
        pumpDelay = emergencyPumpTime;
      }
      else {
        pumpDelay = emergencyPumpTimeRain;
      }
    } while (emergency);

    digitalWrite(ledFail, LOW);
  }
}

void countRotation() {
  if (millis() - lastRotTime > reedDebouce) {
    rotation ++;
    lastReedTime = millis();
    Serial.println(rotation);
  }
  lastRotTime = millis();
  emergency = false;
}

void pumpCycle() {
  Serial.println("PUMP");
  digitalWrite(ledPump, HIGH);
  digitalWrite(pump, HIGH);
  digitalWrite(13, HIGH);
  delay(pumpTime);
  digitalWrite(13, LOW);
  digitalWrite(pump, LOW);
  digitalWrite(ledPump, LOW);
}


