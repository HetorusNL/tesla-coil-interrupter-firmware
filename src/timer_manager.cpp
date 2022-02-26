#include "timer_manager.h"

CoilTimer* TimerManager::timers[NUM_TIMERS] = {0};
bool TimerManager::timersInUse[NUM_TIMERS] = {0};

TimerManager::TimerManager() {
  // create the timer objects
  for (int i = 0; i < NUM_TIMERS; i++) {
    timers[i] = new CoilTimer(i, &hwTimers[i]);
  }
}

TimerManager::~TimerManager() {
  // remove the timer objects
  for (int i = 0; i < NUM_TIMERS; i++) {
    delete timers[i];
    timers[i] = nullptr;
  }
}

// get the first available timer, returns nullptr if all timers are occupied
CoilTimer* TimerManager::getTimer() {
  for (int i = 0; i < NUM_TIMERS; i++) {
    if (!timersInUse[i]) {
      timersInUse[i] = true;
      return timers[i];
    }
  }
  // no timer is available, return nullptr
  return nullptr;
}

// make sure that the timer is stopped and release the timer for future use
void TimerManager::releaseTimer(CoilTimer* timer) {
  for (int i = 0; i < NUM_TIMERS; i++) {
    if (timers[i] == timer) {
      timer->stop();
      timersInUse[i] = false;
    }
  }
}

// release all timers so they can be used for other purposes
void TimerManager::releaseAllTimers() {
  for (int i = 0; i < NUM_TIMERS; i++) {
    if (timersInUse[i]) {
      timers[i]->stop();
      timersInUse[i] = false;
    }
  }
}

void TC0_Handler() {
  TC_GetStatus(TC0, 0);
  TimerManager::timers[0]->createSpark(TimerManager::getNumActiveTimers());
}

void TC1_Handler() {
  TC_GetStatus(TC0, 1);
  TimerManager::timers[1]->createSpark(TimerManager::getNumActiveTimers());
}

void TC2_Handler() {
  TC_GetStatus(TC0, 2);
  TimerManager::timers[2]->createSpark(TimerManager::getNumActiveTimers());
}

void TC3_Handler() {
  TC_GetStatus(TC1, 0);
  TimerManager::timers[3]->createSpark(TimerManager::getNumActiveTimers());
}

void TC4_Handler() {
  TC_GetStatus(TC1, 1);
  TimerManager::timers[4]->createSpark(TimerManager::getNumActiveTimers());
}

void TC5_Handler() {
  TC_GetStatus(TC1, 2);
  TimerManager::timers[5]->createSpark(TimerManager::getNumActiveTimers());
}

void TC6_Handler() {
  TC_GetStatus(TC2, 0);
  TimerManager::timers[6]->createSpark(TimerManager::getNumActiveTimers());
}

void TC7_Handler() {
  TC_GetStatus(TC2, 1);
  TimerManager::timers[7]->createSpark(TimerManager::getNumActiveTimers());
}

void TC8_Handler() {
  TC_GetStatus(TC2, 2);
  TimerManager::timers[8]->createSpark(TimerManager::getNumActiveTimers());
}