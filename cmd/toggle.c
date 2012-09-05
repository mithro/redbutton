/* Copyright (c) 2012, Jan Vaughan
 * All rights reserved.
 *
 * Main application for toggling the usb blinker.
 */

#include <flags.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>

#include "../common.h"

#define BUF_LEN 256

/* Host->Device vendor request to endpoint 0. */
#define EP0_IN (USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_ENDPOINT)
/* Device->Host vendor request to endpoint 0. */
#define EP0_OUT (USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_ENDPOINT)

DEFINE_uint64(vid, 0x16C0, "Vendor ID of the device to connect to.");
DEFINE_uint64(pid, 0x05DC, "Product ID of the device to connect to.");
DEFINE_string(manufacturer, "leaf.net.au", "Vendor description string of the device to connect to."
    " Set to empty if the description should not be checked.");
DEFINE_string(product, "blink", "Product description string of the device to connect to."
    " Set to empty if the description should not be checked.");

DEFINE_bool(read, 0, "Read the current state of the LED before issuing any commands.");
DEFINE_bool(change, 1, "Controls whether any commands should be issued to the device.");
DEFINE_bool(blink, 0, "Tell the device to blink on its own accord rather than toggling the LED.");

DEFINE_uint64(timeout_ms, 1000, "Timeout when talking with the device.");

static void fregister(void) {
  REGISTER(vid);
  REGISTER(pid);
  REGISTER(manufacturer);
  REGISTER(product);
  REGISTER(read);
  REGISTER(change);
  REGISTER(blink);
  REGISTER(timeout_ms);
}

int checkString(usb_dev_handle *dev, int stringIndex, char *buf, char *expected) {
  int len;
  len = usb_get_string_simple(dev, stringIndex, buf, BUF_LEN - 1);
  if (len < 0) {
    return 0;
  }
  if (buf[len]) {
    buf[len + 1] = '\0';
  }
  return !*expected || strncmp(expected, buf, len + 1) == 0;
}

usb_dev_handle* connect(void) {
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *handle = NULL;

  char manufacturer[BUF_LEN];
  char product[BUF_LEN];

  usb_find_busses();
  usb_find_devices();
  for (bus = usb_get_busses(); bus && !handle; bus = bus->next) {
    for (dev = bus->devices; dev && !handle; dev = dev->next) {
      if (dev->descriptor.idVendor == FLAGS_vid && dev->descriptor.idProduct == FLAGS_pid) {
        handle = usb_open(dev);
        if (!handle) {
          fprintf(stderr, "cannot open USB device: %s\n", usb_strerror());
        } else if (
            checkString(handle, dev->descriptor.iManufacturer, manufacturer, FLAGS_manufacturer)
            && checkString(handle, dev->descriptor.iProduct, product, FLAGS_product)) {
          printf("manufacturer: %s\n", *manufacturer ? manufacturer : "");
          printf("product: %s\n", *product ? product : "");
        } else {
          handle = NULL;
        }
      }
    }
  }
  return handle;
}

int main(int argc, char **argv) {
  usb_dev_handle *blinker = NULL;
  int len;
  uint8_t state;

  fregister();
  parse_flags(&argc, &argv);
  usb_init();
  blinker = connect();
  if (!blinker) {
    fprintf(stderr, "No device, aborting.\n");
    exit(1);
  }

  if (FLAGS_read) {
    len = usb_control_msg(
        blinker,
        EP0_OUT,
        LED_STATE_REQ,
        0, /* wValue: unused */
        0, /* wIndex: control request to endpoint 0 */
        (char *) &state, 1,
        FLAGS_timeout_ms);
    if (len < 0) {
      fprintf(stderr, "USB error reading LED state: %s\n", usb_strerror());
    } else if (len == 0) {
      fprintf(stderr, "Device read did not give any bytes.\n");
    } else {
      printf("Read state: LED %s%s\n",
          (state & LED_BRIGHTNESS_MASK) == LED_ON ? "on" : "off",
          (state & LED_MODE_MASK) == BLINK_MODE ? ", blinking" : "");
    }
  }

  if (FLAGS_change) {
    len = usb_control_msg(
        blinker,
        EP0_IN,
        FLAGS_blink ? BLINK_REQ : TOGGLE_REQ,
        0, /* wValue: unused */
        0, /* wIndex: control request to endpoint 0 */
        NULL, 0, /* no data */
        FLAGS_timeout_ms);
    if (len < 0) {
      fprintf(stderr, "USB error issuing command: %s\n", usb_strerror());
    }
  }

  usb_close(blinker);
  return 0;
}
