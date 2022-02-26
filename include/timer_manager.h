#ifndef TimerManager_h
#define TimerManager_h

#include "coil_timer.h"

class TimerManager {
 public:
  static constexpr int NUM_TIMERS = 9;

 private:
  CoilTimer::HwTimer hwTimers[NUM_TIMERS] = {
      {TC0, 0, TC0_IRQn}, {TC0, 1, TC1_IRQn}, {TC0, 2, TC2_IRQn},
      {TC1, 0, TC3_IRQn}, {TC1, 1, TC4_IRQn}, {TC1, 2, TC5_IRQn},
      {TC2, 0, TC6_IRQn}, {TC2, 1, TC7_IRQn}, {TC2, 2, TC8_IRQn},
  };

 public:
  TimerManager();
  ~TimerManager();

  CoilTimer* getTimer();
  void releaseTimer(CoilTimer* timer);
  void releaseAllTimers();

 private:
  inline static uint16_t getNumActiveTimers() {
    uint16_t res = 0;
    for (int i = 0; i < NUM_TIMERS; i++) {
      res += timers[i]->isActive();
    }
    return res;
  }

 protected:
  // make the interrupt handler friends, so they can use internal callbacks
  friend void TC0_Handler(void);
  friend void TC1_Handler(void);
  friend void TC2_Handler(void);
  friend void TC3_Handler(void);
  friend void TC4_Handler(void);
  friend void TC5_Handler(void);
  friend void TC6_Handler(void);
  friend void TC7_Handler(void);
  friend void TC8_Handler(void);

  static CoilTimer* timers[];

 private:
  static bool timersInUse[];
};

#endif  // TimerManager_h