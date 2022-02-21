#include "coil_timer.h"

CoilTimer::CoilTimer(int id, HwTimer* hwTimer)
    : id(id), hwTimer(hwTimer), _frequency(1), _velocity(0), active(false) {
  // configure the timer for the default frequency of 1 (Hz)
  setFrequency(_frequency);
}

CoilTimer::~CoilTimer() {
  // nothing to do here
}

void CoilTimer::start() {
  active = true;
  NVIC_ClearPendingIRQ(hwTimer->irq);
  NVIC_EnableIRQ(hwTimer->irq);
  TC_Start(hwTimer->tc, hwTimer->channel);
}

void CoilTimer::stop() {
  active = false;
  NVIC_DisableIRQ(hwTimer->irq);
  TC_Stop(hwTimer->tc, hwTimer->channel);
}

void CoilTimer::setFrequency(double frequency) {
  // prevent negative and zero frequencies
  if (frequency <= 0) {
    frequency = 1;
  }
  _frequency = frequency;

  uint32_t rc;

  // tell the Power Management Controller to disable the write protection of the
  // (Timer/Counter) registers:
  pmc_set_writeprotect(false);

  // enable clock for the timer
  pmc_enable_periph_clk((uint32_t)hwTimer->irq);

  // we don't need the best clock, since 32bit timer with lowest prescaler (2)
  // is always sufficient for 1Hz or faster timing
  // get the rc for the wanted frequency
  getClock(_frequency, rc);
  constexpr uint8_t clock = TC_CMR_TCCLKS_TIMER_CLOCK1;

  // set up the timer in waveform mode which create a PWM in UP mode with
  // automatic trigger on RC Compare and sets it up with the determined interval
  // clock as clock input
  TC_Configure(hwTimer->tc, hwTimer->channel,
               TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | clock);
  // reset counter and fire interrupt when RC value is matched
  TC_SetRC(hwTimer->tc, hwTimer->channel, rc);
  // enable the RC Compare Interrupt
  hwTimer->tc->TC_CHANNEL[hwTimer->channel].TC_IER = TC_IER_CPCS;
  // and disable all other interrupts
  hwTimer->tc->TC_CHANNEL[hwTimer->channel].TC_IDR = ~TC_IER_CPCS;
}

void CoilTimer::getClock(double& frequency, uint32_t& rc) {
  float ticks = (float)SystemCoreClock / frequency / clockConfig[0].divisor;
  rc = (uint32_t)round(ticks);
  frequency = (double)SystemCoreClock / clockConfig[0].divisor / (double)rc;
}

// pick the best clock
// Timer		Definition
// TIMER_CLOCK1	MCK /  2
// TIMER_CLOCK2	MCK /  8
// TIMER_CLOCK3	MCK / 32
// TIMER_CLOCK4	MCK /128
uint8_t CoilTimer::bestClock(double& frequency, uint32_t& rc) {
  float ticks;
  float error;
  int clkId = 0;
  int bestClock = 3;
  float bestTicks = UINT32_MAX;
  float bestError = 9.999e99;
  do {
    ticks =
        (float)SystemCoreClock / frequency / (float)clockConfig[clkId].divisor;
    // error comparison needs scaling
    error = clockConfig[clkId].divisor * abs(ticks - round(ticks));
    if (error < bestError) {
      bestClock = clkId;
      bestError = error;
      bestTicks = ticks;
    }
    Serial.print("divisor: ");
    Serial.print(clockConfig[clkId].divisor);
    Serial.print(" ticks: ");
    Serial.print(ticks);
    Serial.print(" error: ");
    Serial.println(error);
  } while (clkId++ < 3);
  // update the rc value with the ticks and calculate actual frequency
  rc = (uint32_t)round(bestTicks);
  frequency =
      (double)SystemCoreClock / clockConfig[bestClock].divisor / (double)rc;
  return clockConfig[bestClock].flag;
}

void CoilTimer::createSpark(uint16_t numActiveTimers) {
  // calculate the on duration here
  uint16_t onTime = getOnTime();
  uint16_t duration = onTime;
  // take the number of active timers into account
  double activeTimerFac = (0.5 + (double)numActiveTimers / 2);
  // base the active timer volume adjustment on the frequency
  double volAdjuctedFac =
      1. + (activeTimerFac - 1);  // * (onTimes[0]) / (onTime);
  duration = (uint16_t)((double)duration / volAdjuctedFac);
  // reduce duration by velocity if velocity is low
  duration *= (127 + _velocity / 2);
  duration >>= 8;
  // limit the min/max values of the duration
  if (duration < MIN_ON_TIME) duration = MIN_ON_TIME;
  if (duration > MAX_ON_TIME) duration = MAX_ON_TIME;
  //   static long bla = 0;
  //   bla++;
  //   if (bla % 10 == 0) {
  //     Serial.print(_frequency);
  //     Serial.println(duration);
  //   }
  // apply the spark for duration (us) ON and then duration (us) OFF
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(COIL_PIN, HIGH);
  delayMicroseconds(duration);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(COIL_PIN, LOW);
  delayMicroseconds(duration);
}
