#include "timer_manager.h"

CoilTimer* TimerManager::timers[NUM_TIMERS] = {0};
bool TimerManager::timer_in_use[NUM_TIMERS] = {0};

TimerManager::TimerManager() {
    // create the timer objects
    for (int i = 0; i < NUM_TIMERS; i++) {
        timers[i] = new CoilTimer(i, &timer_config[i]);
    }

    // Give USART3 (Serial) the highest priority
    NVIC_SetPriority(USART3_IRQn, 1);  // Lower number = higher priority

    // Set lower priority for all 9 timers
    NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5);   // TIM1
    NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 5);   // TIM8
    NVIC_SetPriority(TIM3_IRQn, 5);            // TIM3
    NVIC_SetPriority(TIM4_IRQn, 5);            // TIM4
    NVIC_SetPriority(TIM6_DAC_IRQn, 5);        // TIM6
    NVIC_SetPriority(TIM7_IRQn, 5);            // TIM7
    NVIC_SetPriority(TIM2_IRQn, 5);            // TIM2 (32-bit)
    NVIC_SetPriority(TIM5_IRQn, 5);            // TIM5 (32-bit)
    NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 5);  // TIM12
}

TimerManager::~TimerManager() {
    // remove the timer objects
    for (int i = 0; i < NUM_TIMERS; i++) {
        delete timers[i];
        timers[i] = nullptr;
    }
}

// get the first available timer, returns nullptr if all timers are occupied
CoilTimer* TimerManager::get_timer() {
    for (int i = 0; i < NUM_TIMERS; i++) {
        if (!timer_in_use[i]) {
            timer_in_use[i] = true;
            return timers[i];
        }
    }
    // no timer is available, return nullptr
    return nullptr;
}

// make sure that the timer is stopped and release the timer for future use
void TimerManager::release_timer(CoilTimer* timer) {
    for (int i = 0; i < NUM_TIMERS; i++) {
        if (timers[i] == timer) {
            timer->stop();
            timer_in_use[i] = false;
        }
    }
}

// release all timers so they can be used for other purposes
void TimerManager::release_all_timers() {
    for (int i = 0; i < NUM_TIMERS; i++) {
        if (timer_in_use[i]) {
            timers[i]->stop();
            timer_in_use[i] = false;
        }
    }
}

PHC::PACKET_HANDLE_RESULT TimerManager::perform_reset() {
    // reset this manager by releasing all timers
    release_all_timers();
    return PHC::PACKET_HANDLE_RESULT::RESULT_OK;
}
