#pragma once

#include <Arduino.h>

#define COIL_PIN 10

class CoilTimer {
public:
    // hardware_timer related properties
    struct Config {
        TIM_TypeDef* type_def;
        IRQn_Type irq_type;
    };

public:
    CoilTimer(int id, Config* config);
    ~CoilTimer();

    CoilTimer* start();
    CoilTimer* stop();
    CoilTimer* set_frequency(double frequency);
    inline void set_velocity(uint8_t velocity) { _velocity = velocity; };
    void create_spark(uint16_t num_active_timers);

    inline double get_frequency() { return _frequency; }
    inline bool is_active() { return active; }

    void isr();

private:
    inline uint8_t get_on_time() {
        // make sure that the type of the array is a single uint8_t, or change sizeof
        uint16_t index = (uint16_t)_frequency / 100;
        if (index > sizeof(on_times))
            return 0;
        uint16_t base = on_times[index];
        uint16_t interval = on_times[index] - on_times[index + 1];
        return base - interval * (((uint16_t)_frequency % 100) + 1) / 100;
    }
    bool is32bit() const {
        // STM32F7: TIM2 and TIM5 are 32-bit
        return (config->type_def == TIM2 || config->type_def == TIM5);
    }
    uint32_t get_arr_limit() const { return is32bit() ? 0xFFFFFFFF : 0xFFFF; }

private:
    HardwareTimer* hardware_timer;
    const int id;
    Config* config;  // pointer to config set in constructor, no deletion here
    double _frequency;
    uint8_t _velocity;
    bool active;
    bool spark_enabled;

    static constexpr int MIN_ON_TIME = 10;
    static constexpr int MAX_ON_TIME = 100;
    const uint8_t on_times[20] = {193, 133, 98, 84, 74, 63, 55, 50, 45, 42, 38, 34, 30, 27, 24, 22, 20, 18, 17, 16};
};
