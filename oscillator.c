/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Calibrates the internal RC oscillator of chips such as the ATtiny45 to
 * 16.5MHz using USB as a reference.
 */

#include "oscillator.h"

#include <avr/eeprom.h>

#include "usbdrv.h"

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 * Note: This calibration algorithm may try OSCCAL values of up to 192 even if
 * the optimum value is far below 192. It may therefore exceed the allowed clock
 * frequency of the CPU in low voltage designs!
 * You may replace this search algorithm with any other algorithm you like if
 * you have additional constraints such as a maximum CPU clock.
 * For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
 * ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
 * both regions.
 */
void calibrateOscillator(void) {
  uchar step = 128;
  uchar trialValue = 0;
  uchar optimumValue;
  int x;
  int optimumDev;
  int targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

  /* Do a binary search: */
  do {
    OSCCAL = trialValue + step;
    x = usbMeasureFrameLength(); /* Proportional to current real frequency. */
    if (x < targetValue) {
      /* Frequency still too low. */
      trialValue += step;
    }
    step >>= 1;
  } while (step > 0);
  /* We have a precision of +/- 1 for optimum OSCCAL, now do a neighbourhood
   * search for the optimum value. */
  optimumValue = trialValue;
  optimumDev = x; /* This is certainly far away from optimum. */
  for (OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++) {
    x = usbMeasureFrameLength() - targetValue;
    if (x < 0) {
        x = -x;
    }
    if (x < optimumDev) {
      optimumDev = x;
      optimumValue = OSCCAL;
    }
  }
  OSCCAL = optimumValue;
}

void calibrateAndSaveOscillator(void) {
  calibrateOscillator();
  eeprom_write_byte(EEPROM_OSCCAL_BYTE, OSCCAL);
}

void restoreOscillator(void) {
  uchar calibrationValue = eeprom_read_byte(EEPROM_OSCCAL_BYTE);
  if(calibrationValue != 0xff){
    OSCCAL = calibrationValue;
  }
}
