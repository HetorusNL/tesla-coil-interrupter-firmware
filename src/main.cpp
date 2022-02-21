#include <Arduino.h>

#include "midi_controller.h"
#include "timer_manager.h"
#include "utils.h"

// user pointer and initialize after Serial to prevent Serial breaky breaky
MidiController* midiController = nullptr;
TimerManager* timerManager = nullptr;
CoilTimer* firstTimer = nullptr;
CoilTimer* testTimer = nullptr;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(COIL_PIN, OUTPUT);
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
  Serial.begin(115200);
  timerManager = new TimerManager();
  midiController = new MidiController();
  firstTimer = timerManager->getTimer();
  firstTimer->setFrequency(440);
  testTimer = timerManager->getTimer();
  // clear the Serial buffer
  while (Serial.available()) Serial.read();
}

char* midiMsg = new char[5];
int midiIndex = 0;
uint32_t freq = 0;
bool midiMode = false;

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'm') {
      // we start doing MIDI stuff, next readlines should be forwarded to
      // MidiController
      midiMode = true;
      midiIndex = 0;
    }
    if (c == '\n') {
      if (midiMode) {
        midiIndex = 0;
        for (int i = 0; i < 5; i++) {
          debugprint((int)midiMsg[i]);
          debugprint(" ");
        }
        debugprintln();
        if (!midiController->processMessage(midiMsg)) {
          // if above returned false, exit from midi mode
          midiMode = false;
        }
        Serial.println("ACK");
      } else {
        if (freq < 1) {
          Serial.println();
          Serial.println("stopping 2nd timer");
          testTimer->stop();
        } else {
          if (freq > 20000) {
            // we don't do higher frequencies
            freq = 20000;
          }
          Serial.println();
          Serial.print("setting 2nd timer frequency to: ");
          Serial.println(freq);
          testTimer->setFrequency(freq);
          testTimer->start();
        }
        freq = 0;
      }
    } else if (c == '\r') {
      // ignore the pesky \r characters
    } else {
      if (midiMode) {
        if (midiIndex < 5) midiMsg[midiIndex] = c;
        midiIndex++;
      } else {
        if (c == 'y') {
          firstTimer->start();
        } else if (c == 'n') {
          firstTimer->stop();
        } else if (c >= '0' && c <= '9') {
          freq *= 10;
          freq += c - '0';
          Serial.print(c - '0');
        }
      }
    }
  }
}
