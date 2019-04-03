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
const int LOOP_START_LED_PIN = 5;

int currentStep = 1;
int steps = 8;                   // how many steps before sequencer loop resets
int clocked = 0;                 // how many times externally clocked
long pulseDuration = 20000;      // how long, in microseconds, the clock led will be lit
long stepDuration = 500000;      // how long, in microseconds, a single step lasts before the next step begins. Will be variable based on clock input
long timeOfLoopStart = 0;        // when the first step occured on the system clock

// RECORDING TOUCH SEQUENCE
byte seqEventStatesA = 0;        // records a total of 8 touch events (if 1 == there was an event, 0 == no event)
long seqEventDurationsA[8];      // records the duration of each event (mapped to seqEventStatesA)

// MICROSECOND RECORDERS
long timeOfLastClock = 0;        //
long timeOfLastTouchA = 0;       // when channel A was last touched in microseconds
long timeOfLastReleaseA = 0;     // when channel A was last released in microseconds
bool triggered = false;          // determin if channel A has already been triggered/set HIGH

uint16_t lastTouched = 0;
uint16_t currTouched = 0;

const int CHANNEL_A_LED_PIN = 8;   // via io

// Timer1 Interupt Callback
void advanceClock() {

  timeOfLastClock = micros();  // the current time
  if (currentStep == 1) {
    timeOfLoopStart = timeOfLastClock;
    Serial.print("timeOfLoopStart: ");Serial.println(timeOfLoopStart);
  }
  // increment currentStep by 1
  if (currentStep < steps) {
    currentStep += 1;
  } else {
    currentStep = 1;
  }
}


void setup() {
  Serial.begin(9600);
  // Connect MPR121 touch sensors
  if (!touch.begin(TOUCH_ADDR)) { Serial.println("MPR121 not found, check wiring?"); while (1); }

  io.begin(IO_ADDR);

  Timer1.initialize(stepDuration); // initialize @ 120 BPM
  Timer1.attachInterrupt(advanceClock);

  pinMode(CLOCK_LED_PIN, OUTPUT);
  digitalWrite(CLOCK_LED_PIN, LOW);
  pinMode(LOOP_START_LED_PIN, OUTPUT);
  digitalWrite(LOOP_START_LED_PIN, LOW);

  io.pinMode(CHANNEL_A_LED_PIN, OUTPUT);
  io.digitalWrite(CHANNEL_A_LED_PIN, LOW);
  Serial.println("::: Setup Complete :::");
}

void loop() {

  long now = micros();

  if (now - pulseDuration < timeOfLastClock) {
    digitalWrite(CLOCK_LED_PIN, HIGH);
    if (currentStep == 1) {digitalWrite(LOOP_START_LED_PIN, HIGH);}
  } else {
    digitalWrite(CLOCK_LED_PIN, LOW);
    if (currentStep == 1) {digitalWrite(LOOP_START_LED_PIN, LOW);}
  }


  // Get the currently touched pads
  currTouched = touch.touched();

  // Iterate over touch sensors
  for (uint8_t i=0; i<4; i++) {

    // if it *is* touched and *wasnt* touched before
    if ( (currTouched & _BV(i) ) && !( lastTouched & _BV(i) ) ) {
      io.digitalWrite(CHANNEL_A_LED_PIN, HIGH);

      // get the current time of touch relative to timeOfLoopStart
      timeOfLastTouchA = now - timeOfLoopStart;

      Serial.print("timeOfLastTouchA: ");Serial.println(timeOfLastTouchA);
    }

    //  if it *was* touched and now *isnt*
    if (!(currTouched & _BV(i)) && (lastTouched & _BV(i)) ) {
      io.digitalWrite(CHANNEL_A_LED_PIN, LOW);
      timeOfLastReleaseA = now - timeOfLoopStart;

      for (uint8_t i = 0; i < 8; i++) {
        // record an event and assign the events duration to array
        if (bitRead(seqEventStatesA, i) == LOW) {
          bitWrite(seqEventStatesA, i, HIGH);
          seqEventDurationsA[i] = timeOfLastReleaseA - timeOfLastTouchA;
          break;
        }
      }

      Serial.print("timeOfLastReleaseA: ");Serial.println(timeOfLastReleaseA);
    }
  }

  // if now is greater than lastTouch and less than lastRelease
  if (now - timeOfLoopStart > timeOfLastTouchA && now - timeOfLoopStart < timeOfLastReleaseA && !triggered) {
    io.digitalWrite(CHANNEL_A_LED_PIN, HIGH);
    triggered = true;
  } else if (now - timeOfLoopStart > timeOfLastReleaseA && triggered) {
    io.digitalWrite(CHANNEL_A_LED_PIN, LOW);
    triggered = false;
  }

  // reset touch sensors state
  lastTouched = currTouched;

}
