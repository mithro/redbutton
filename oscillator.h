/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Functions for calibrating the 16.5MHz oscillator for use with VUSB.
 * USB_RESET_HOOK and USB_CFG_HAVE_MEASURE_FRAME_LENGTH must both be enabled in
 * the application's usbconfig.h.
 */

#ifndef __oscillator_h_included__
#define __oscillator_h_included__

/* EEPROM byte in which oscillator calibration is preserved over resets. */
#define EEPROM_OSCCAL_BYTE 0

/* Calibrate the RC oscillator to 8.25MHz, giving a core clock of 16.5MHz.
 * Note: This calibration algorithm may try OSCCAL values of up to 192 even if
 * the optimum value is far below 192. It may therefore exceed the allowed clock
 * frequency of the CPU in low voltage designs!
 */
void calibrateOscillator(void);

/* As per calibrateOscillator(), but also saves the calibrated value to EEPROM.
 * Suitable for connecting directly to USB_RESET_HOOK.
 */
void calibrateAndSaveOscillator(void);

/* Restore oscillator calibration to a value saved in EEPROM. */
void restoreOscillator(void);

#endif /* __oscillator_h_included__ */
