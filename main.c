/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Main application.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "common.h"
#include "oscillator.h"
#include "usbdrv.h"

/* Pin assignments:
 * PB0 = unused input (pulled high)
 * PB1 = button input (pulled high)
 * PB2 = USB data
 * PB3 = USB data
 * PB4 = LED output (active high)
 */
#define LED (1 << 4)

/* USB Endpoint 0 (control):
 *   H>D TOGGLE: Stop blinking. Switch LED from current state.
 *   H>D BLINK: Turn LED off, then automatically blink on for 500ms of every 2s.
 *   D>H LED_STATE: Read whether LED is on or off and blinking or fixed. Sends
 *       one byte defined by the masks and values in common.h.
 */


/* Tracks whether blinking is enabled and time into the blink cycle. 255 is disabled, otherwise
 * counts from 0 to 200 centiseconds then loops. LED is enabled from 150 to 200. */
static uint8_t blinking = 255;

/* Byte containing response data for sending out over USB. */
static uint8_t resp;

static usbMsgLen_t handleHostToDevice(usbRequest_t *req);
static usbMsgLen_t handleDeviceToHost(usbRequest_t *req);
static void blink(void);


USB_PUBLIC usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
  usbRequest_t *req = (usbRequest_t *)data;

  /* We only handle vendor requests involing endpoint 0. We don't expect class requests since we're
   * a vendor specific device, and vusb should have handled standard type requests for us. */
  if ((req->bmRequestType & (USBRQ_TYPE_MASK | USBRQ_RCPT_MASK))
      != (USBRQ_TYPE_VENDOR | USBRQ_RCPT_ENDPOINT)
      || (req->wIndex.bytes[0] & 0x0f) != 0) {
    return 0;
  }
  if ((req->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_HOST_TO_DEVICE) {
    return handleHostToDevice(req);
  } else {
    return handleDeviceToHost(req);
  }
}

static usbMsgLen_t handleHostToDevice(usbRequest_t *req) {
  if (req->bRequest == TOGGLE_REQ) {
    blinking = 255;
    PORTB ^= LED;
  } else if (req->bRequest == BLINK_REQ) {
    PORTB &= ~LED;
    blinking = 0;
  }
  return 0;
}

static usbMsgLen_t handleDeviceToHost(usbRequest_t *req) {
  if (req->bRequest == LED_STATE_REQ) {
    resp = ((PORTB & LED) ? LED_ON : LED_OFF)
        | (blinking == 255 ? FIXED_MODE : BLINK_MODE);
    usbMsgPtr = &resp;
    /* Both USB and AVR words are little endian. */
    return req->wLength.word >= 1 ? 1 : 0;
  } else {
    return 0;
  }
}

static void blink(void) {
  blinking++;
  if (blinking == 150) {
    PORTB |= LED;
  } else if (blinking >= 200) {
    PORTB &= ~LED;
    blinking = 0;
  }
}

int main(void) {
  uint8_t i;

  /* LED is output, all other pins are inputs. */
  DDRB = LED;
  /* Enable pullups on inputs other than the USB ones, LED on to start. */
  PORTB = ~USBMASK;

  restoreOscillator();

  /* Disconnect USB 300ms to reset, then connect. */
  usbDeviceDisconnect();
  for (i = 0; i < 20; i++) {
    _delay_ms(15);
  }
  usbDeviceConnect();

  /* Enable USB */
  usbInit();
  sei();

  /* Ready to go, turn off LED and await instruction. */
  PORTB &= ~LED;
  while (1) {
    usbPoll();
    _delay_ms(10);
    if (blinking != 255) {
      blink();
    }
  }

  return 0; /* never reached */
}
