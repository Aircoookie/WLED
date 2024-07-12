/* esp8266_waveform imported from platform source code
   Modified for WLED to work around a fault in the NMI handling,
   which can result in the system locking up and hard WDT crashes.

   Imported from https://github.com/esp8266/Arduino/blob/7e0d20e2b9034994f573a236364e0aef17fd66de/cores/esp8266/core_esp8266_waveform_pwm.cpp
*/

/*
  esp8266_waveform - General purpose waveform generation and control,
                     supporting outputs on all pins in parallel.

  Copyright (c) 2018 Earle F. Philhower, III.  All rights reserved.

  The core idea is to have a programmable waveform generator with a unique
  high and low period (defined in microseconds or CPU clock cycles).  TIMER1
  is set to 1-shot mode and is always loaded with the time until the next
  edge of any live waveforms.

  Up to one waveform generator per pin supported.

  Each waveform generator is synchronized to the ESP clock cycle counter, not
  the timer.  This allows for removing interrupt jitter and delay as the
  counter always increments once per 80MHz clock.  Changes to a waveform are
  contiguous and only take effect on the next waveform transition,
  allowing for smooth transitions.

  This replaces older tone(), analogWrite(), and the Servo classes.

  Everywhere in the code where "cycles" is used, it means ESP.getCycleCount()
  clock cycle count, or an interval measured in CPU clock cycles, but not
  TIMER1 cycles (which may be 2 CPU clock cycles @ 160MHz).

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include <Arduino.h>
#include <coredecls.h>
#include "ets_sys.h"
#include "core_esp8266_waveform.h"
#include "user_interface.h"

extern "C" {

// Linker magic
void usePWMFixedNMI() {};

// Maximum delay between IRQs
#define MAXIRQUS (10000)

// Waveform generator can create tones, PWM, and servos
typedef struct {
  uint32_t nextServiceCycle;   // ESP cycle timer when a transition required
  uint32_t expiryCycle;        // For time-limited waveform, the cycle when this waveform must stop
  uint32_t timeHighCycles;     // Actual running waveform period (adjusted using desiredCycles)
  uint32_t timeLowCycles;      //
  uint32_t desiredHighCycles;  // Ideal waveform period to drive the error signal
  uint32_t desiredLowCycles;   //
  uint32_t lastEdge;           // Cycle when this generator last changed
} Waveform;

class WVFState {
public:
  Waveform waveform[17];        // State of all possible pins
  uint32_t waveformState = 0;   // Is the pin high or low, updated in NMI so no access outside the NMI code
  uint32_t waveformEnabled = 0; // Is it actively running, updated in NMI so no access outside the NMI code

  // Enable lock-free by only allowing updates to waveformState and waveformEnabled from IRQ service routine
  uint32_t waveformToEnable = 0;  // Message to the NMI handler to start a waveform on a inactive pin
  uint32_t waveformToDisable = 0; // Message to the NMI handler to disable a pin from waveform generation

  uint32_t waveformToChange = 0; // Mask of pin to change. One bit set in main app, cleared when effected in the NMI
  uint32_t waveformNewHigh = 0;
  uint32_t waveformNewLow = 0;

  uint32_t (*timer1CB)() = NULL;

  // Optimize the NMI inner loop by keeping track of the min and max GPIO that we
  // are generating.  In the common case (1 PWM) these may be the same pin and
  // we can avoid looking at the other pins.
  uint16_t startPin = 0;
  uint16_t endPin = 0;
};
static WVFState wvfState;


// Ensure everything is read/written to RAM
#define MEMBARRIER() { __asm__ volatile("" ::: "memory"); }

// Non-speed critical bits
#pragma GCC optimize ("Os")

// Interrupt on/off control
static IRAM_ATTR void timer1Interrupt();
static bool timerRunning = false;

static __attribute__((noinline)) void initTimer() {
  if (!timerRunning) {
    timer1_disable();
    ETS_FRC_TIMER1_INTR_ATTACH(NULL, NULL);
    ETS_FRC_TIMER1_NMI_INTR_ATTACH(timer1Interrupt);
    timer1_enable(TIM_DIV1, TIM_EDGE, TIM_SINGLE);
    timerRunning = true;
    timer1_write(microsecondsToClockCycles(10));
  }
}

static IRAM_ATTR void forceTimerInterrupt() {
  if (T1L > microsecondsToClockCycles(10)) {
    T1L = microsecondsToClockCycles(10);
  }
}

// PWM implementation using special purpose state machine
//
// Keep an ordered list of pins with the delta in cycles between each
// element, with a terminal entry making up the remainder of the PWM
// period.  With this method sum(all deltas) == PWM period clock cycles.
//
// At t=0 set all pins high and set the timeout for the 1st edge.
// On interrupt, if we're at the last element reset to t=0 state
// Otherwise, clear that pin down and set delay for next element
// and so forth.

constexpr int maxPWMs = 8;

// PWM machine state
typedef struct PWMState {
  uint32_t mask; // Bitmask of active pins
  uint32_t cnt;  // How many entries
  uint32_t idx;  // Where the state machine is along the list
  uint8_t  pin[maxPWMs + 1];
  uint32_t delta[maxPWMs + 1];
  uint32_t nextServiceCycle;  // Clock cycle for next step
  struct PWMState *pwmUpdate; // Set by main code, cleared by ISR
} PWMState;

static PWMState pwmState;
static uint32_t _pwmFreq = 1000;
static uint32_t _pwmPeriod = microsecondsToClockCycles(1000000UL) / _pwmFreq;


// If there are no more scheduled activities, shut down Timer 1.
// Otherwise, do nothing.
static IRAM_ATTR void disableIdleTimer() {
 if (timerRunning && !wvfState.waveformEnabled && !pwmState.cnt && !wvfState.timer1CB) {
    ETS_FRC_TIMER1_NMI_INTR_ATTACH(NULL);
    timer1_disable();
    timer1_isr_init();
    timerRunning = false;
  }
}

// Notify the NMI that a new PWM state is available through the mailbox.
// Wait for mailbox to be emptied (either busy or delay() as needed)
static IRAM_ATTR void _notifyPWM(PWMState *p, bool idle) {
  p->pwmUpdate = nullptr;
  pwmState.pwmUpdate = p;
  MEMBARRIER();
  forceTimerInterrupt();
  while (pwmState.pwmUpdate) {
    if (idle) {
      esp_yield();
    }
    MEMBARRIER();
  }
}

static void _addPWMtoList(PWMState &p, int pin, uint32_t val, uint32_t range);


// Called when analogWriteFreq() changed to update the PWM total period
//extern void _setPWMFreq_weak(uint32_t freq) __attribute__((weak)); 
void _setPWMFreq_weak(uint32_t freq) {
  _pwmFreq = freq;

  // Convert frequency into clock cycles
  uint32_t cc = microsecondsToClockCycles(1000000UL) / freq;

  // Simple static adjustment to bring period closer to requested due to overhead
  // Empirically determined as a constant PWM delay and a function of the number of PWMs
#if F_CPU == 80000000
  cc -= ((microsecondsToClockCycles(pwmState.cnt) * 13) >> 4) + 110;
#else
  cc -= ((microsecondsToClockCycles(pwmState.cnt) * 10) >> 4) + 75;
#endif

  if (cc == _pwmPeriod) {
    return; // No change
  }

  _pwmPeriod = cc;

  if (pwmState.cnt) {
    PWMState p;  // The working copy since we can't edit the one in use
    p.mask = 0;
    p.cnt = 0;
    for (uint32_t i = 0; i < pwmState.cnt; i++) {
      auto pin = pwmState.pin[i];
      _addPWMtoList(p, pin, wvfState.waveform[pin].desiredHighCycles, wvfState.waveform[pin].desiredLowCycles);
    }
    // Update and wait for mailbox to be emptied
    initTimer();
    _notifyPWM(&p, true);
    disableIdleTimer();
  }
}
/*
static void _setPWMFreq_bound(uint32_t freq) __attribute__((weakref("_setPWMFreq_weak")));
void _setPWMFreq(uint32_t freq) { 
  _setPWMFreq_bound(freq);
}
*/

// Helper routine to remove an entry from the state machine
// and clean up any marked-off entries
static void _cleanAndRemovePWM(PWMState *p, int pin) {
  uint32_t leftover = 0;
  uint32_t in, out;
  for (in = 0, out = 0; in < p->cnt; in++) {
    if ((p->pin[in] != pin) && (p->mask & (1<<p->pin[in]))) {
        p->pin[out] = p->pin[in];
        p->delta[out] = p->delta[in] + leftover;
        leftover = 0;
        out++;
    } else {
        leftover += p->delta[in];
        p->mask &= ~(1<<p->pin[in]);
    }
  }
  p->cnt = out;
  // Final pin is never used: p->pin[out] = 0xff;
  p->delta[out] = p->delta[in] + leftover;
}


// Disable PWM on a specific pin (i.e. when a digitalWrite or analogWrite(0%/100%))
//extern bool _stopPWM_weak(uint8_t pin) __attribute__((weak));
IRAM_ATTR bool _stopPWM_weak(uint8_t pin) {
  if (!((1<<pin) & pwmState.mask)) {
    return false; // Pin not actually active
  }

  PWMState p;  // The working copy since we can't edit the one in use
  p = pwmState;

  // In _stopPWM we just clear the mask but keep everything else
  // untouched to save IRAM.  The main startPWM will handle cleanup.
  p.mask &= ~(1<<pin);
  if (!p.mask) {
    // If all have been stopped, then turn PWM off completely
    p.cnt = 0;
  }

  // Update and wait for mailbox to be emptied, no delay (could be in ISR)
  _notifyPWM(&p, false);
  // Possibly shut down the timer completely if we're done
  disableIdleTimer();
  return true;
}
/*
static bool _stopPWM_bound(uint8_t pin) __attribute__((weakref("_stopPWM_weak")));
IRAM_ATTR bool _stopPWM(uint8_t pin) {
  return _stopPWM_bound(pin);
}
*/

static void _addPWMtoList(PWMState &p, int pin, uint32_t val, uint32_t range) {
  // Stash the val and range so we can re-evaluate the fraction
  // should the user change PWM frequency.  This allows us to
  // give as great a precision as possible.  We know by construction
  // that the waveform for this pin will be inactive so we can borrow
  // memory from that structure.
  wvfState.waveform[pin].desiredHighCycles = val;  // Numerator == high
  wvfState.waveform[pin].desiredLowCycles = range; // Denominator == low

  uint32_t cc = (_pwmPeriod * val) / range;

  // Clip to sane values in the case we go from OK to not-OK when adjusting frequencies
  if (cc == 0) {
    cc = 1;
  } else if (cc >= _pwmPeriod) {
    cc = _pwmPeriod - 1;
  }

  if (p.cnt == 0) {
    // Starting up from scratch, special case 1st element and PWM period
    p.pin[0] = pin;
    p.delta[0] = cc;
   // Final pin is never used: p.pin[1] = 0xff;
    p.delta[1] = _pwmPeriod - cc;
  } else {
    uint32_t ttl = 0;
    uint32_t i;
    // Skip along until we're at the spot to insert
    for (i=0; (i <= p.cnt) && (ttl + p.delta[i] < cc); i++) {
      ttl += p.delta[i];
    }
    // Shift everything out by one to make space for new edge
    for (int32_t j = p.cnt; j >= (int)i; j--) {
      p.pin[j + 1] = p.pin[j];
      p.delta[j + 1] = p.delta[j];
    }
    int off = cc - ttl; // The delta from the last edge to the one we're inserting
    p.pin[i] = pin;
    p.delta[i] = off; // Add the delta to this new pin
    p.delta[i + 1] -= off; // And subtract it from the follower to keep sum(deltas) constant
  }
  p.cnt++;
  p.mask |= 1<<pin;
}

// Called by analogWrite(1...99%) to set the PWM duty in clock cycles
//extern bool _setPWM_weak(int pin, uint32_t val, uint32_t range) __attribute__((weak));
bool _setPWM_weak(int pin, uint32_t val, uint32_t range) {
  stopWaveform(pin);
  PWMState p;  // Working copy
  p = pwmState;
  // Get rid of any entries for this pin
  _cleanAndRemovePWM(&p, pin);
  // And add it to the list, in order
  if (p.cnt >= maxPWMs) {
    return false; // No space left
  }

  // Sanity check for all-on/off
  uint32_t cc = (_pwmPeriod * val) / range;
  if ((cc == 0) || (cc >= _pwmPeriod)) {
    digitalWrite(pin, cc ? HIGH : LOW);
    return true;
  }

  _addPWMtoList(p, pin, val, range);

  // Set mailbox and wait for ISR to copy it over
  initTimer();
  _notifyPWM(&p, true);
  disableIdleTimer();

  // Potentially recalculate the PWM period if we've added another pin
  _setPWMFreq(_pwmFreq);

  return true;
}
/*
static bool _setPWM_bound(int pin, uint32_t val, uint32_t range) __attribute__((weakref("_setPWM_weak")));
bool _setPWM(int pin, uint32_t val, uint32_t range) {
  return _setPWM_bound(pin, val, range);
}
*/

// Start up a waveform on a pin, or change the current one.  Will change to the new
// waveform smoothly on next low->high transition.  For immediate change, stopWaveform()
// first, then it will immediately begin.
//extern int startWaveformClockCycles_weak(uint8_t pin, uint32_t timeHighCycles, uint32_t timeLowCycles, uint32_t runTimeCycles, int8_t alignPhase, uint32_t phaseOffsetUS, bool autoPwm)  __attribute__((weak));
int startWaveformClockCycles_weak(uint8_t pin, uint32_t timeHighCycles, uint32_t timeLowCycles, uint32_t runTimeCycles,
                             int8_t alignPhase, uint32_t phaseOffsetUS, bool autoPwm) {
  (void) alignPhase;
  (void) phaseOffsetUS;
  (void) autoPwm;

   if ((pin > 16) || isFlashInterfacePin(pin) || (timeHighCycles == 0)) {
    return false;
  }
  Waveform *wave = &wvfState.waveform[pin];
  wave->expiryCycle = runTimeCycles ? ESP.getCycleCount() + runTimeCycles : 0;
  if (runTimeCycles && !wave->expiryCycle) {
    wave->expiryCycle = 1; // expiryCycle==0 means no timeout, so avoid setting it
  }

  _stopPWM(pin); // Make sure there's no PWM live here

  uint32_t mask = 1<<pin;
  MEMBARRIER();
  if (wvfState.waveformEnabled & mask) {
    // Make sure no waveform changes are waiting to be applied
    while (wvfState.waveformToChange) {
      esp_yield(); // Wait for waveform to update
      MEMBARRIER();
    }
    wvfState.waveformNewHigh = timeHighCycles;
    wvfState.waveformNewLow = timeLowCycles;
    MEMBARRIER();
    wvfState.waveformToChange = mask;
    // The waveform will be updated some time in the future on the next period for the signal
  } else { //  if (!(wvfState.waveformEnabled & mask)) {
    wave->timeHighCycles = timeHighCycles;
    wave->desiredHighCycles = timeHighCycles;
    wave->timeLowCycles = timeLowCycles;
    wave->desiredLowCycles = timeLowCycles;
    wave->lastEdge = 0;
    wave->nextServiceCycle = ESP.getCycleCount() + microsecondsToClockCycles(1);
    wvfState.waveformToEnable |= mask;
    MEMBARRIER();
    initTimer();
    forceTimerInterrupt();
    while (wvfState.waveformToEnable) {
      esp_yield(); // Wait for waveform to update
      MEMBARRIER();
    }
  }

  return true;
}
/*
static int startWaveformClockCycles_bound(uint8_t pin, uint32_t timeHighCycles, uint32_t timeLowCycles, uint32_t runTimeCycles, int8_t alignPhase, uint32_t phaseOffsetUS, bool autoPwm) __attribute__((weakref("startWaveformClockCycles_weak")));
int startWaveformClockCycles(uint8_t pin, uint32_t timeHighCycles, uint32_t timeLowCycles, uint32_t runTimeCycles, int8_t alignPhase, uint32_t phaseOffsetUS, bool autoPwm) {
  return startWaveformClockCycles_bound(pin, timeHighCycles, timeLowCycles, runTimeCycles, alignPhase, phaseOffsetUS, autoPwm);
}


// This version falls-thru to the proper startWaveformClockCycles call and is invariant across waveform generators
int startWaveform(uint8_t pin, uint32_t timeHighUS, uint32_t timeLowUS, uint32_t runTimeUS,
                  int8_t alignPhase, uint32_t phaseOffsetUS, bool autoPwm) {
  return startWaveformClockCycles_bound(pin,
    microsecondsToClockCycles(timeHighUS), microsecondsToClockCycles(timeLowUS),
    microsecondsToClockCycles(runTimeUS), alignPhase, microsecondsToClockCycles(phaseOffsetUS), autoPwm);
}
*/

// Set a callback.  Pass in NULL to stop it
//extern void setTimer1Callback_weak(uint32_t (*fn)()) __attribute__((weak));
void setTimer1Callback_weak(uint32_t (*fn)()) {
  wvfState.timer1CB = fn;
  if (fn) {
    initTimer();
    forceTimerInterrupt();
  }
  disableIdleTimer();
}
/*
static void setTimer1Callback_bound(uint32_t (*fn)()) __attribute__((weakref("setTimer1Callback_weak")));
void setTimer1Callback(uint32_t (*fn)()) {
  setTimer1Callback_bound(fn);
}
*/

// Stops a waveform on a pin
//extern int stopWaveform_weak(uint8_t pin) __attribute__((weak));
IRAM_ATTR int stopWaveform_weak(uint8_t pin) {
  // Can't possibly need to stop anything if there is no timer active
  if (!timerRunning) {
    return false;
  }
  // If user sends in a pin >16 but <32, this will always point to a 0 bit
  // If they send >=32, then the shift will result in 0 and it will also return false
  uint32_t mask = 1<<pin;
  if (wvfState.waveformEnabled & mask) {
    wvfState.waveformToDisable = mask;
    // Cancel any pending updates for this waveform, too.
    if (wvfState.waveformToChange & mask) {
        wvfState.waveformToChange = 0;
    }
    forceTimerInterrupt();
    while (wvfState.waveformToDisable) {
      MEMBARRIER(); // If it wasn't written yet, it has to be by now
      /* no-op */ // Can't delay() since stopWaveform may be called from an IRQ
    }
  }
  disableIdleTimer();
  return true;
}
/*
static int stopWaveform_bound(uint8_t pin) __attribute__((weakref("stopWaveform_weak")));
IRAM_ATTR int stopWaveform(uint8_t pin) {
  return stopWaveform_bound(pin);
}
*/

// Speed critical bits
#pragma GCC optimize ("O2")

// Normally would not want two copies like this, but due to different
// optimization levels the inline attribute gets lost if we try the
// other version.
static inline IRAM_ATTR uint32_t GetCycleCountIRQ() {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a"(ccount));
  return ccount;
}

// Find the earliest cycle as compared to right now
static inline IRAM_ATTR uint32_t earliest(uint32_t a, uint32_t b) {
    uint32_t now = GetCycleCountIRQ();
    int32_t da = a - now;
    int32_t db = b - now;
    return (da < db) ? a : b;
}

// ----- @willmmiles begin patch -----
// NMI crash workaround
// Sometimes the NMI fails to return, stalling the CPU.  When this happens,
// the next NMI gets a return address /inside the NMI handler function/.
// We work around this by caching the last NMI return address, and restoring
// the epc3 and eps3 registers to the previous values if the observed epc3
// happens to be pointing to the _NMILevelVector function.
extern void _NMILevelVector();
extern void _UserExceptionVector_1(); // the next function after _NMILevelVector
static inline IRAM_ATTR void nmiCrashWorkaround() {
  static uintptr_t epc3_backup, eps3_backup;

  uintptr_t epc3, eps3;
  __asm__ __volatile__("rsr %0,epc3; rsr %1,eps3":"=a"(epc3),"=a" (eps3));
  if ((epc3 < (uintptr_t) &_NMILevelVector) || (epc3 >= (uintptr_t) &_UserExceptionVector_1)) {
    // Address is good; save backup
    epc3_backup = epc3;
    eps3_backup = eps3;
  } else {
    // Address is inside the NMI handler -- restore from backup
    __asm__ __volatile__("wsr %0,epc3; wsr %1,eps3"::"a"(epc3_backup),"a"(eps3_backup));
  }
}
// ----- @willmmiles end patch -----


// The SDK and hardware take some time to actually get to our NMI code, so
// decrement the next IRQ's timer value by a bit so we can actually catch the
// real CPU cycle counter we want for the waveforms.

// The SDK also sometimes is running at a different speed the the Arduino core
// so the ESP cycle counter is actually running at a variable speed.
// adjust(x) takes care of adjusting a delta clock cycle amount accordingly.
#if F_CPU == 80000000
  #define DELTAIRQ (microsecondsToClockCycles(9)/4)
  #define adjust(x) ((x) << (turbo ? 1 : 0))
#else
  #define DELTAIRQ (microsecondsToClockCycles(9)/8)
  #define adjust(x) ((x) >> 0)
#endif

// When the time to the next edge is greater than this, RTI and set another IRQ to minimize CPU usage
#define MINIRQTIME microsecondsToClockCycles(6)

static IRAM_ATTR void timer1Interrupt() {
  // ----- @willmmiles begin patch -----
  nmiCrashWorkaround();
  // ----- @willmmiles end patch -----

  // Flag if the core is at 160 MHz, for use by adjust()
  bool turbo = (*(uint32_t*)0x3FF00014) & 1 ? true : false;

  uint32_t nextEventCycle = GetCycleCountIRQ() + microsecondsToClockCycles(MAXIRQUS);
  uint32_t timeoutCycle = GetCycleCountIRQ() + microsecondsToClockCycles(14);

  if (wvfState.waveformToEnable || wvfState.waveformToDisable) {
    // Handle enable/disable requests from main app
    wvfState.waveformEnabled = (wvfState.waveformEnabled & ~wvfState.waveformToDisable) | wvfState.waveformToEnable; // Set the requested waveforms on/off
    wvfState.waveformState &= ~wvfState.waveformToEnable;  // And clear the state of any just started
    wvfState.waveformToEnable = 0;
    wvfState.waveformToDisable = 0;
    // No mem barrier.  Globals must be written to RAM on ISR exit.
    // Find the first GPIO being generated by checking GCC's find-first-set (returns 1 + the bit of the first 1 in an int32_t)
    wvfState.startPin = __builtin_ffs(wvfState.waveformEnabled) - 1;
    // Find the last bit by subtracting off GCC's count-leading-zeros (no offset in this one)
    wvfState.endPin = 32 - __builtin_clz(wvfState.waveformEnabled);
  } else if (!pwmState.cnt && pwmState.pwmUpdate) {
    // Start up the PWM generator by copying from the mailbox
    pwmState.cnt = 1;
    pwmState.idx = 1; // Ensure copy this cycle, cause it to start at t=0
    pwmState.nextServiceCycle = GetCycleCountIRQ(); // Do it this loop!
    // No need for mem barrier here.  Global must be written by IRQ exit
  }

  bool done = false;
  if (wvfState.waveformEnabled || pwmState.cnt) {
    do {
      nextEventCycle = GetCycleCountIRQ() + microsecondsToClockCycles(MAXIRQUS);

      // PWM state machine implementation
      if (pwmState.cnt) {
        int32_t cyclesToGo;
        do {
            cyclesToGo = pwmState.nextServiceCycle - GetCycleCountIRQ();
            if (cyclesToGo < 0) {
                if (pwmState.idx == pwmState.cnt) { // Start of pulses, possibly copy new
                  if (pwmState.pwmUpdate) {
                    // Do the memory copy from temp to global and clear mailbox
                    pwmState = *(PWMState*)pwmState.pwmUpdate;
                  }
                  GPOS = pwmState.mask; // Set all active pins high
                  if (pwmState.mask & (1<<16)) {
                    GP16O = 1;
                  }
                  pwmState.idx = 0;
                } else {
                  do {
                    // Drop the pin at this edge
                    if (pwmState.mask & (1<<pwmState.pin[pwmState.idx])) {
                      GPOC = 1<<pwmState.pin[pwmState.idx];
                      if (pwmState.pin[pwmState.idx] == 16) {
                        GP16O = 0;
                      }
                    }
                    pwmState.idx++;
                    // Any other pins at this same PWM value will have delta==0, drop them too.
                  } while (pwmState.delta[pwmState.idx] == 0);
                }
                // Preserve duty cycle over PWM period by using now+xxx instead of += delta
                cyclesToGo = adjust(pwmState.delta[pwmState.idx]);
                pwmState.nextServiceCycle = GetCycleCountIRQ() + cyclesToGo;
            }
            nextEventCycle = earliest(nextEventCycle, pwmState.nextServiceCycle);
        } while (pwmState.cnt && (cyclesToGo < 100));
      }

      for (auto i = wvfState.startPin; i <= wvfState.endPin; i++) {
        uint32_t mask = 1<<i;

        // If it's not on, ignore!
        if (!(wvfState.waveformEnabled & mask)) {
          continue;
        }

        Waveform *wave = &wvfState.waveform[i];
        uint32_t now = GetCycleCountIRQ();

        // Disable any waveforms that are done
        if (wave->expiryCycle) {
          int32_t expiryToGo = wave->expiryCycle - now;
          if (expiryToGo < 0) {
              // Done, remove!
              if (i == 16) {
                GP16O = 0;
              } 
              GPOC = mask;
              wvfState.waveformEnabled &= ~mask;
              continue;
            }
        }

        // Check for toggles
        int32_t cyclesToGo = wave->nextServiceCycle - now;
        if (cyclesToGo < 0) {
          uint32_t nextEdgeCycles;
          uint32_t desired = 0;
          uint32_t *timeToUpdate;
          wvfState.waveformState ^= mask;
          if (wvfState.waveformState & mask) {
            if (i == 16) {
              GP16O = 1;
            }
            GPOS = mask;

            if (wvfState.waveformToChange & mask) {
              // Copy over next full-cycle timings
              wave->timeHighCycles = wvfState.waveformNewHigh;
              wave->desiredHighCycles = wvfState.waveformNewHigh;
              wave->timeLowCycles = wvfState.waveformNewLow;
              wave->desiredLowCycles = wvfState.waveformNewLow;
              wave->lastEdge = 0;
              wvfState.waveformToChange = 0;
            }
            if (wave->lastEdge) {
              desired = wave->desiredLowCycles;
              timeToUpdate = &wave->timeLowCycles;
            }
            nextEdgeCycles = wave->timeHighCycles;
          } else {
            if (i == 16) {
              GP16O = 0;
            }
            GPOC = mask;
            desired = wave->desiredHighCycles;
            timeToUpdate = &wave->timeHighCycles;
            nextEdgeCycles = wave->timeLowCycles;
          }
          if (desired) {
            desired = adjust(desired);
            int32_t err = desired - (now - wave->lastEdge);
            if (abs(err) < desired) { // If we've lost > the entire phase, ignore this error signal
                err /= 2;
                *timeToUpdate += err;
            }
          }
          nextEdgeCycles = adjust(nextEdgeCycles);
          wave->nextServiceCycle = now + nextEdgeCycles;
          wave->lastEdge = now;
        }
        nextEventCycle = earliest(nextEventCycle, wave->nextServiceCycle);
      }

      // Exit the loop if we've hit the fixed runtime limit or the next event is known to be after that timeout would occur
      uint32_t now = GetCycleCountIRQ();
      int32_t cycleDeltaNextEvent = nextEventCycle - now;
      int32_t cyclesLeftTimeout = timeoutCycle - now;
      done = (cycleDeltaNextEvent > MINIRQTIME) || (cyclesLeftTimeout < 0);
    } while (!done);
  } // if (wvfState.waveformEnabled)

  if (wvfState.timer1CB) {
    nextEventCycle = earliest(nextEventCycle, GetCycleCountIRQ() + wvfState.timer1CB());
  }

  int32_t nextEventCycles = nextEventCycle - GetCycleCountIRQ();

  if (nextEventCycles < MINIRQTIME) {
    nextEventCycles = MINIRQTIME;
  }
  nextEventCycles -= DELTAIRQ;

  // Do it here instead of global function to save time and because we know it's edge-IRQ
  T1L = nextEventCycles >> (turbo ? 1 : 0);
}

};
