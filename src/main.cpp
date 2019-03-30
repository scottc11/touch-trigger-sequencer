#include <Arduino.h>
#include <Adafruit_MCP23017.h>
#include <TimerOne.h>
#include <Adafruit_MPR121.h>

#define IO_ADDR 0x00
#define TOUCH_ADDR 0x5A

Adafruit_MCP23017 io = Adafruit_MCP23017();
Adafruit_MPR121 touch = Adafruit_MPR121();

const bool DEBUG = false;
const int CLOCK_LED_PIN = 4;

int currentStep = 1;       //
int steps = 8;             // how many steps before sequencer loop resets
int clocked = 0;           // how many times externally clocked
long pulseLength = 20000;  // how long, in microseconds, the clock led will be lit

long timeOfLastClock = 0;  //

uint16_t lastTouched = 0;
uint16_t currTouched = 0;



// Timer1 Interupt Callback
void advanceClock() {

  timeOfLastClock = micros();  // the current time

  // increment currentStep by 1
  if (currentStep < steps) {
    currentStep += 1;
  }

  if (currentStep == steps) {
    currentStep = 1;
  }
  Serial.print("current step: ");Serial.println(currentStep);
  Serial.print("time of clock: ");Serial.println(timeOfLastClock);
}


void setup() {
  Serial.begin(9600);
  // Connect MPR121 touch sensors
  if (!touch.begin(TOUCH_ADDR)) { Serial.println("MPR121 not found, check wiring?"); while (1); }

  io.begin(IO_ADDR);

  Timer1.initialize(500000); // initialize @ 120 BPM
  Timer1.attachInterrupt(advanceClock);

  pinMode(CLOCK_LED_PIN, OUTPUT);
  digitalWrite(CLOCK_LED_PIN, LOW);
  Serial.println("::: Setup Complete :::");
}

void loop() {

  //
  long now = micros();

  if (now - pulseLength < timeOfLastClock) {
    digitalWrite(CLOCK_LED_PIN, HIGH);
  } else {
    digitalWrite(CLOCK_LED_PIN, LOW);
  }


  // Get the currently touched pads
  currTouched = touch.touched();

  // Iterate over touch sensors
  for (uint8_t i=0; i<12; i++) {

    // if it *is* touched and *wasnt* touched before
    if ( (currTouched & _BV(i) ) && !( lastTouched & _BV(i) ) ) {
      Serial.println("boop");
    }

    //  if it *was* touched and now *isnt*
    if (!(currTouched & _BV(i)) && (lastTouched & _BV(i)) ) {
      Serial.println("beep");
    }
  }

  // reset touch sensors state
  lastTouched = currTouched;

}
