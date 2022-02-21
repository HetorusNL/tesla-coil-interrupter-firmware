#ifndef MidiController_h
#define MidiController_h

#include "timer_manager.h"

class MidiController {
 public:
  MidiController();
  ~MidiController();

  // processes a 3-char midi message
  bool processMessage(char* msg);

 private:
  void noteOn(char* msg);
  void noteOff(char* msg);

 private:
  TimerManager timerManager;
  struct MidiTone {
    bool used;
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
    uint16_t delay;
    CoilTimer* coilTimer;
  };
  MidiTone midiTones[TimerManager::NUM_TIMERS];
  double midiFrequency[128];  // precompute MIDI frequencies
};

#endif  // MidiController_h