/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * USB constants relevant to both device and driver.
 */

#ifndef __usb_blink__common_h_included__
#define __usb_blink__common_h_included__

/* Host->Device request codes */
#define TOGGLE_REQ 0
#define BLINK_REQ 1

/* Device->Host request codes */
#define LED_STATE_REQ 0

/* LED state message */
#define LED_BRIGHTNESS_MASK 1
#define LED_ON LED_BRIGHTNESS_MASK
#define LED_OFF 0
#define LED_MODE_MASK (1 << 1)
#define FIXED_MODE 0
#define BLINK_MODE LED_MODE_MASK

#endif /* __usb_blink__common_h_included__ */
