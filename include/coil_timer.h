#ifndef CoilTimer_h
#define CoilTimer_h

#include <Arduino.h>

#define COIL_PIN 10

typedef void (*voidFuncPtr)(void);

class CoilTimer {
public:
    // timer related properties
    struct HwTimer {
        TIM_TypeDef* timer;
        voidFuncPtr isr;
    };

public:
    CoilTimer(int id, HwTimer* hwTimer);
    ~CoilTimer();
    CoilTimer* start();
    CoilTimer* stop();
    CoilTimer* setFrequency(double frequency);
    inline void setVelocity(uint8_t velocity) { _velocity = velocity; };
    void createSpark(uint16_t numActiveTimers);

    inline double getFrequency() { return _frequency; }
    inline bool isActive() { return active; }

private:
    void getClock(double& frequency, uint32_t& rc);
    inline byte getOnTime() {
        // make sure that the type of the array is a single byte, or change sizeof
        uint16_t index = (uint16_t)_frequency / 100;
        if (index > sizeof(onTimes))
            return 0;
        uint16_t base = onTimes[index];
        uint16_t interval = onTimes[index] - onTimes[index + 1];
        return base - interval * (((uint16_t)_frequency % 100) + 1) / 100;
    }
    bool is32bit() const {
        // STM32F7: TIM2 and TIM5 are 32-bit
        return (hwTimer->timer == TIM2 || hwTimer->timer == TIM5);
    }

    uint32_t getARRLimit() const { return is32bit() ? 0xFFFFFFFF : 0xFFFF; }

private:
    HardwareTimer* timer;
    const int id;
    HwTimer* hwTimer;  // pointer to hwTimer set in constructor, no deletion here
    double _frequency;
    uint8_t _velocity;
    bool active;

    static constexpr int MIN_ON_TIME = 10;
    static constexpr int MAX_ON_TIME = 100;
    const byte onTimes[20] = {193, 133, 98, 84, 74, 63, 55, 50, 45, 42, 38, 34, 30, 27, 24, 22, 20, 18, 17, 16};

    struct ClockConfig {
        uint8_t flag;
        uint8_t divisor;
    };
};

#endif  // CoilTimer_h