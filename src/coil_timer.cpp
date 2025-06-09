#include "coil_timer.h"
#include <HardwareTimer.h>

CoilTimer::CoilTimer(int id, HwTimer* hwTimer) : id(id), hwTimer(hwTimer), _frequency(1), _velocity(0), active(false) {
    timer = new HardwareTimer(hwTimer->timer);
    // configure the timer for the default frequency of 1 (Hz)
    setFrequency(_frequency);
}

CoilTimer::~CoilTimer() {
    // nothing to do here
}

CoilTimer* CoilTimer::start() {
    if (isActive())
        return this;
    active = true;
    timer->resume();
    return this;
}

CoilTimer* CoilTimer::stop() {
    if (!isActive())
        return this;
    active = false;
    timer->pause();
    return this;
}

CoilTimer* CoilTimer::setFrequency(double frequency) {
    uint32_t clk = timer->getTimerClkFreq();
    uint32_t maxARR = getARRLimit();
    bool wide = is32bit();
    uint32_t bestPSC = 0, bestARR = 0;
    float bestErr = 1.0f;
    const float maxRelErr = 0.001f;  // ±0.1%
    uint32_t maxPSC = wide ? 0xFFFFFFFF : 0xFFFF;

    for (uint32_t psc = 0; psc <= maxPSC; ++psc) {
        float arr_f = (clk / ((psc + 1.0f) * frequency)) - 1.0f;
        if (arr_f < 0.0f)
            break;
        uint32_t arr = (uint32_t)(arr_f + 0.5f);
        if (arr > maxARR)
            continue;

        float realHz = clk / ((psc + 1.0f) * (arr + 1.0f));
        float err = fabsf((realHz - frequency) / frequency);
        if (err < bestErr) {
            bestErr = err;
            bestPSC = psc;
            bestARR = arr;
            if (err < maxRelErr)
                break;
        }
    }

    if (bestErr > maxRelErr) {
        Serial.print("timer has too large error:");
        Serial.print(bestErr);
        while (true) {
            digitalWrite(LED_RED, true);
            delay(500);
            digitalWrite(LED_RED, false);
            delay(500);
        }
    }

    bool was_active = active;

    if (was_active)
        timer->pause();

    timer->detachInterrupt();
    timer->setPrescaleFactor(bestPSC + 1);
    timer->setOverflow(bestARR, TICK_FORMAT);
    timer->attachInterrupt(hwTimer->isr);
    timer->refresh();

    if (was_active)
        timer->resume();

    _frequency = clk / ((bestPSC + 1.0f) * (bestARR + 1.0f));
    return this;
}

int count = 1;
void CoilTimer::createSpark(uint16_t numActiveTimers) {
    // Serial.println(count++);
    // calculate the on duration here
    uint16_t onTime = getOnTime();
    uint16_t duration = onTime;
    // take the number of active timers into account
    double activeTimerFac = (0.5 + (double)numActiveTimers / 2);
    // base the active timer volume adjustment on the frequency
    double volAdjuctedFac = 1. + (activeTimerFac - 1);  // * (onTimes[0]) / (onTime);
    volAdjuctedFac = 1.5;
    duration = (uint16_t)((double)duration / volAdjuctedFac);
    // reduce duration by velocity if velocity is low
    duration *= (127 + _velocity / 2);
    duration >>= 8;
    // limit the min/max values of the duration
    if (duration < MIN_ON_TIME)
        duration = MIN_ON_TIME;
    if (duration > MAX_ON_TIME)
        duration = MAX_ON_TIME;
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
