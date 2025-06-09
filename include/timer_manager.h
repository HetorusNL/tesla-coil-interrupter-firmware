#ifndef TimerManager_h
#define TimerManager_h

#include "coil_timer.h"
#include "packet_handler_constants.h"

void timerISR0();
void timerISR1();
void timerISR2();
void timerISR3();
void timerISR4();
void timerISR5();
void timerISR6();
void timerISR7();
void timerISR8();

class TimerManager {
public:
    static constexpr int NUM_TIMERS = 9;

private:
    CoilTimer::HwTimer hwTimers[NUM_TIMERS] = {
        {TIM1, timerISR0}, {TIM2, timerISR1}, {TIM3, timerISR2}, {TIM4, timerISR3},  {TIM5, timerISR4},
        {TIM6, timerISR5}, {TIM7, timerISR6}, {TIM8, timerISR7}, {TIM12, timerISR8},
    };

public:
    TimerManager();
    ~TimerManager();

    CoilTimer* getTimer();
    void releaseTimer(CoilTimer* timer);
    void releaseAllTimers();
    PHC::PACKET_HANDLE_RESULT performReset();

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
    friend void timerISR0(void);
    friend void timerISR1(void);
    friend void timerISR2(void);
    friend void timerISR3(void);
    friend void timerISR4(void);
    friend void timerISR5(void);
    friend void timerISR6(void);
    friend void timerISR7(void);
    friend void timerISR8(void);

    static CoilTimer* timers[];

private:
    static bool timersInUse[];
};

#endif  // TimerManager_h