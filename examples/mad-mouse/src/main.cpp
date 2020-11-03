//------------------------------------------------------------------------------

#include <Arduino.h>
#include <Streaming.h>

//------------------------------------------------------------------------------

#define DEBUG_DEMO 1

// -----------------------------------------------------------------------------
// HID
// -----------------------------------------------------------------------------

enum {
  HID_REPORT_KEYBOARD = 1,
  HID_REPORT_MOUSE,
};

// keyboard report
typedef struct {
  uint8_t report_id;
  uint8_t modifier; // bit flags for shift, ctrl, alt etc
  uint8_t reserved;
  uint8_t key[5];   // keys
} hid_keyboard_report_t;

// mouse report
typedef struct {
  uint8_t report_id;
  uint8_t buttons;
  int8_t dx;
  int8_t dy;
} hid_mouse_report_t;

static union {
  hid_keyboard_report_t kr;
  hid_mouse_report_t mr;
} hid_report;

static uint8_t hid_led_state = 0xff;

// -----------------------------------------------------------------------------
// USB
// -----------------------------------------------------------------------------

#include <usbdrv.h>

//
// USB-HID descriptor for combo (keyboard + mouse) device
//
PROGMEM const uchar usbHidReportDescriptor[] = {
  //
  // keyboard ------
  //
  0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
  0x09, 0x06,       // USAGE (Keyboard)
  0xa1, 0x01,       // COLLECTION (Application)
  0x85, HID_REPORT_KEYBOARD, // REPORT_ID (1)
  0x75, 0x01,       //   REPORT_SIZE (1)
  0x95, 0x08,       //   REPORT_COUNT (8)
  0x05, 0x07,       //   USAGE_PAGE (Keyboard)(Key Codes)
  0x19, 0xe0,       //   USAGE_MINIMUM (Keyboard LeftControl)(224)
  0x29, 0xe7,       //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
  0x15, 0x00,       //   LOGICAL_MINIMUM (0)
  0x25, 0x01,       //   LOGICAL_MAXIMUM (1)
  0x81, 0x02,       //   INPUT (Data,Var,Abs) ; Modifier byte
  0x95, 0x01,       //   REPORT_COUNT (1)
  0x75, 0x08,       //   REPORT_SIZE (8)
  0x81, 0x03,       //   INPUT (Cnst,Var,Abs) ; Reserved byte
  0x95, 0x05,       //   REPORT_COUNT (5)
  0x75, 0x01,       //   REPORT_SIZE (1)
  0x05, 0x08,       //   USAGE_PAGE (LEDs)
  0x19, 0x01,       //   USAGE_MINIMUM (Num Lock)
  0x29, 0x05,       //   USAGE_MAXIMUM (Kana)
  0x91, 0x02,       //   OUTPUT (Data,Var,Abs) ; LED report
  0x95, 0x01,       //   REPORT_COUNT (1)
  0x75, 0x03,       //   REPORT_SIZE (3)
  0x91, 0x03,       //   OUTPUT (Cnst,Var,Abs) ; LED report padding
  0x95, 0x05,       //   REPORT_COUNT (5)
  0x75, 0x08,       //   REPORT_SIZE (8)
  0x15, 0x00,       //   LOGICAL_MINIMUM (0)
  0x26, 0xA4, 0x00, //   LOGICAL_MAXIMUM (164)
  0x05, 0x07,       //   USAGE_PAGE (Keyboard)(Key Codes)
  0x19, 0x00,       //   USAGE_MINIMUM (Reserved (no event indicated))(0)
  0x2A, 0xA4, 0x00, //   USAGE_MAXIMUM (Keyboard Application)(164)
  0x81, 0x00,       //   INPUT (Data,Ary,Abs)
  0xc0,             // END_COLLECTION
  //
  // mouse ---------
  //
  0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
  0x09, 0x02,       // USAGE (Mouse)
  0xa1, 0x01,       // COLLECTION (Application)
  0x09, 0x01,       //   USAGE (Pointer)
  0xA1, 0x00,       //   COLLECTION (Physical)
  0x85, HID_REPORT_MOUSE, // REPORT_ID (2)
  0x05, 0x09,       //     USAGE_PAGE (Button)
  0x19, 0x01,       //     USAGE_MINIMUM
  0x29, 0x03,       //     USAGE_MAXIMUM
  0x15, 0x00,       //     LOGICAL_MINIMUM (0)
  0x25, 0x01,       //     LOGICAL_MAXIMUM (1)
  0x95, 0x03,       //     REPORT_COUNT (3)
  0x75, 0x01,       //     REPORT_SIZE (1)
  0x81, 0x02,       //     INPUT (Data,Var,Abs)
  0x95, 0x01,       //     REPORT_COUNT (1)
  0x75, 0x05,       //     REPORT_SIZE (5)
  0x81, 0x03,       //     INPUT (Const,Var,Abs)
  0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
  0x09, 0x30,       //     USAGE (X)
  0x09, 0x31,       //     USAGE (Y)
  0x15, 0x81,       //     LOGICAL_MINIMUM (-127)
  0x25, 0x7F,       //     LOGICAL_MAXIMUM (127)
  0x75, 0x08,       //     REPORT_SIZE (8)
  0x95, 0x02,       //     REPORT_COUNT (2)
  0x81, 0x06,       //     INPUT (Data,Var,Rel)
  0xC0,             //   END_COLLECTION
  0xC0,             // END COLLECTION
};

// -----------------------------------------------------------------------------

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
  // dummy just to obey USB-HID
  static uint8_t hid_idle_rate;

  // only interested in class requests
  usbRequest_t *rq = (usbRequest_t *)data;
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS) return 0;

  usbMsgLen_t len = 0;

  // dispatch requests
  switch (rq->bRequest) {
    // host requests a report from device (8 bytes at most for USB 1.1?)
    case USBRQ_HID_GET_REPORT:
      // which report?
      switch (rq->wValue.bytes[0]) {
        // keyboard
        case HID_REPORT_KEYBOARD:
          // just setup pointer to buffer and its length
          usbMsgPtr = (uchar *)&hid_report;
          len = sizeof(hid_report.kr);
          break;
        // mouse
        case HID_REPORT_MOUSE:
          // just setup pointer to buffer and its length
          usbMsgPtr = (uchar *)&hid_report;
          len = sizeof(hid_report.mr);
          break;
      }
      break;
    // host sends a report to device (8 bytes at most for USB 1.1?)
    case USBRQ_HID_SET_REPORT:
      // which report?
      switch (rq->wValue.bytes[0]) {
        // keyboard
        case HID_REPORT_KEYBOARD:
          // LED state? use usbFunctionWrite() to receive data from host
          if (rq->wLength.word == 2) len = USB_NO_MSG;
          break;
      }
      break;
    // get/set idle rate, not used here
    case USBRQ_HID_GET_IDLE:
      usbMsgPtr = &hid_idle_rate;
      len = 1;
      break;
    case USBRQ_HID_SET_IDLE:
      hid_idle_rate = rq->wValue.bytes[1];
      break;
  }

  return len;
}

// -----------------------------------------------------------------------------

// host sent data to device
USB_PUBLIC usbMsgLen_t usbFunctionWrite(uint8_t *data, uint8_t len)
{
  // LED state reported by host
  hid_led_state = data[1];
  return len;
}

// -----------------------------------------------------------------------------
// DEMO stuff
// -----------------------------------------------------------------------------

#if DEBUG_DEMO

#include "hid.h"

static void hid_keyboard_event(uint8_t c)
{
  hid_report.kr.report_id = HID_REPORT_KEYBOARD;
  hid_report.kr.modifier = 0;
#if 0
  if (c >= 'a' && c <= 'z') {
    hid_report.kr.key[0] = 4 + (c - 'a');
  } else {
    hid_report.kr.key[0] = 0;
  }
#else
  hid_report.kr.key[0] = c;
#endif
}

static void hid_mouse_event(void)
{
  static int rand = 1234;
  hid_report.mr.report_id = HID_REPORT_MOUSE;
  rand=(rand*109+89)%251;
  hid_report.mr.dx = (rand&0xf)-8;
  hid_report.mr.dy = ((rand&0xf0)>>4)-8;
}

static uint8_t demo_text[] = {
  KEYCODE_H, 0, KEYCODE_E, 0, KEYCODE_L, 0, KEYCODE_L, 0, KEYCODE_O, 0, KEYCODE_BACKSPACE, 0, KEYCODE_BACKSPACE, 0, KEYCODE_BACKSPACE, 0, KEYCODE_BACKSPACE, 0, KEYCODE_BACKSPACE, 0,
  //'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 0,
};

#else

static uint8_t hid_report_len;

#endif

//------------------------------------------------------------------------------

void setup()
{

  // init USB
  usbInit();
  usbDeviceDisconnect();
  for (auto i = 0; i < 250; ++i) { /*wdt_reset();*/ _delay_ms(2); }
  usbDeviceConnect();

}

//------------------------------------------------------------------------------

void loop()
{

  // process USB frames
  usbPoll();
  // USB ready?
#if DEBUG_DEMO
  if (usbInterruptIsReady()) {
    static uint8_t what = 0;
    static int i = 0;
    if (what++ % 2) {
      hid_mouse_event();
      usbSetInterrupt((uint8_t *)&hid_report, sizeof(hid_mouse_report_t));
    } else {
      hid_keyboard_event(demo_text[i]);
      usbSetInterrupt((uint8_t *)&hid_report, sizeof(hid_keyboard_report_t));
      if (++i == sizeof(demo_text)) i = 0;
    }
  }
#else
  // TODO: inmplement queue filled by frame_process and emptied here
  if (usbInterruptIsReady()) {
    if (hid_report_len) {
      usbSetInterrupt((uint8_t *)&hid_report, hid_report_len);
      hid_report_len = 0;
    }
  }
#endif

}

//------------------------------------------------------------------------------
