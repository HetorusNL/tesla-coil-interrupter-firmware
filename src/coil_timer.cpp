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
  if (isActive()) return;
  active = true;
  // set the priority of the timer interrupt to 1 (0 is default and highest
  // priority) so it doesn't disturb the UART interrupt!!
  NVIC_SetPriority(hwTimer->irq, 1);
  NVIC_ClearPendingIRQ(hwTimer->irq);
  NVIC_EnableIRQ(hwTimer->irq);
  TC_Start(hwTimer->tc, hwTimer->channel);
}

void CoilTimer::stop() {
  if (!isActive()) return;
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

void CoilTimer::createSpark(uint16_t numActiveTimers) {
  // calculate the on duration here
  uint16_t onTime = getOnTime();
  uint16_t duration = onTime;
  // take the number of active timers into account
  double activeTimerFac = (0.5 + (double)numActiveTimers / 2);
  // base the active timer volume adjustment on the frequency
  double volAdjuctedFac =
      1. + (activeTimerFac - 1);  // * (onTimes[0]) / (onTime);
  volAdjuctedFac = 1.5;
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
