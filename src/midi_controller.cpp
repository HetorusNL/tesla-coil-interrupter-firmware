#include "midi_controller.h"

#include "utils.h"

using PACKET_HANDLE_RESULT = PHC::PACKET_HANDLE_RESULT;

constexpr float MidiController::midiFrequency[];

MidiController::MidiController() : timerManager(), midiTones{0} {}

MidiController::~MidiController() {
  // nothing to do here
}

PACKET_HANDLE_RESULT MidiController::StartStream(uint8_t* data, uint8_t len) {
  // release the timers before starting the MIDI playback
  timerManager.releaseAllTimers();
  return PACKET_HANDLE_RESULT::RESULT_OK;
}

PACKET_HANDLE_RESULT MidiController::HandleMessage(uint8_t* data, uint8_t len) {
  // TODO add queueing functionality here
  processMessage(data);
  return PACKET_HANDLE_RESULT::RESULT_OK;
}

PACKET_HANDLE_RESULT MidiController::EndStream(uint8_t* data, uint8_t len) {
  // release the timers to make sure all sounds are stopped
  timerManager.releaseAllTimers();
  return PACKET_HANDLE_RESULT::RESULT_OK;
}

bool MidiController::processMessage(uint8_t* msg) {
  debugprintln((int)msg[0]);
  if ((msg[0] & 0xf0) == 0x90) {
    if (msg[2] == 0) {
      //   Serial.println("note on with velocity zero");
      noteOff(msg);
    } else {
      //   Serial.println("note on");
      noteOn(msg);
    }
  } else if ((msg[0] & 0xf0) == 0x80) {
    // Serial.println("note off");
    noteOff(msg);
  } else if (msg[0] == 0xff) {
    // Serial.println("exit cmd");
    // the ending command, exit midi mode
    return false;
  } else {
    // Serial.println("unknown cmd");
  }
  return true;
}

void MidiController::noteOn(uint8_t* msg) {
  // try to fetch a timer from TimerManager first
  CoilTimer* timer = timerManager.getTimer();
  if (!timer) {
    return;
  }
  // see if there is a MidiTone available to use for this note
  for (int i = 0; i < TimerManager::NUM_TIMERS; i++) {
    if (!midiTones[i].used) {
      midiTones[i].used = true;
      midiTones[i].channel = msg[0] & 0xf;
      midiTones[i].note = msg[1];
      midiTones[i].velocity = msg[2];
      midiTones[i].delay = (uint16_t)msg[3] << 8 | (uint16_t)msg[4];
      debugprint("delay: ");
      debugprintln(midiTones[i].delay);
      midiTones[i].coilTimer = timer;
      midiTones[i].coilTimer->setFrequency(midiFrequency[midiTones[i].note]);
      midiTones[i].coilTimer->start();
      return;
    }
  }
}

void MidiController::noteOff(uint8_t* msg) {
  // find the note that should be stopped
  for (int i = 0; i < TimerManager::NUM_TIMERS; i++) {
    if (midiTones[i].used && midiTones[i].channel == (msg[0] & 0xf) &&
        midiTones[i].note == msg[1]) {
      // found the node, now stop the timer and release the MidiTone
      debugprint("delay: ");
      debugprintln(midiTones[i].delay);
      midiTones[i].coilTimer->stop();
      timerManager.releaseTimer(midiTones[i].coilTimer);
      midiTones[i].used = false;
      return;
    }
  }
  Serial.println("note not found!");
  for (int i = 0; i < TimerManager::NUM_TIMERS; i++) {
    debugprint(i);
    debugprint(": ");
    debugprint(midiTones[i].used);
    debugprint(", ");
    debugprint(midiTones[i].channel);
    debugprint(", ");
    debugprintln(midiTones[i].note);
  }
  debugprint(midiTones[4].used);
  debugprint(" ");
  debugprint(midiTones[4].channel);
  debugprint(" == ");
  debugprint(msg[0] & 0xf);
  debugprint("; ");
  debugprint(midiTones[4].note);
  debugprint(" == ");
  debugprint(msg[1]);
  debugprint("; ");
  debugprint(midiTones[4].note == msg[1]);
  debugprint(" with cast: ");
  debugprint(midiTones[4].note == (uint8_t)msg[1]);
  debugprint("; ");
  debugprintln(midiTones[4].channel == (msg[0] & 0xf));
}
