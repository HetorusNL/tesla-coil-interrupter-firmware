#ifndef CoilTimer_h
#define CoilTimer_h

#include <Arduino.h>

#define COIL_PIN 10

class CoilTimer {
 public:
  // timer related properties
  struct HwTimer {
    Tc* tc;
    uint32_t channel;
    IRQn_Type irq;
  };

 public:
  CoilTimer(int id, HwTimer* hwTimer);
  ~CoilTimer();
  void start();
  void stop();
  void setFrequency(double frequency);
  inline void setVelocity(uint8_t velocity) { _velocity = velocity; };
  void createSpark(uint16_t numActiveTimers);

  inline double getFrequency() { return _frequency; }
  inline bool isActive() { return active; }

 private:
  void getClock(double& frequency, uint32_t& rc);
  uint8_t bestClock(double& frequency, uint32_t& rc);
  inline byte getOnTime() {
    // make sure that the type of the array is a single byte, or change sizeof
    uint16_t index = (uint16_t)_frequency / 100;
    if (index > sizeof(onTimes)) return 0;
    uint16_t base = onTimes[index];
    uint16_t interval = onTimes[index] - onTimes[index + 1];
    return base - interval * (((uint16_t)_frequency % 100) + 1) / 100;
  }

 private:
  const int id;
  HwTimer* hwTimer;  // pointer to hwTimer set in constructor, no deletion here
  double _frequency;
  uint8_t _velocity;
  bool active;

  static constexpr int MIN_ON_TIME = 10;
  static constexpr int MAX_ON_TIME = 100;
  const byte onTimes[20] = {193, 133, 98, 84, 74, 63, 55, 50, 45, 42,
                            38,  34,  30, 27, 24, 22, 20, 18, 17, 16};

  struct ClockConfig {
    uint8_t flag;
    uint8_t divisor;
  };
  const ClockConfig clockConfig[4] = {{TC_CMR_TCCLKS_TIMER_CLOCK1, 2},
                                      {TC_CMR_TCCLKS_TIMER_CLOCK2, 8},
                                      {TC_CMR_TCCLKS_TIMER_CLOCK3, 32},
                                      {TC_CMR_TCCLKS_TIMER_CLOCK4, 128}};
};

#endif  // CoilTimer_h