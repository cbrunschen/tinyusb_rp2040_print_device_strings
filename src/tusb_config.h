#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef CFG_TUSB_DEBUG
#undef CFG_TUSB_DEBUG
#endif

#define CFG_TUSB_DEBUG 2

#define CFG_TUSB_OS OPT_OS_PICO

// Enable host stack
#define CFG_TUH_ENABLED 1

#define BOARD_TUH_RHPORT 1
#define CFG_TUSB_RHPORT1_MODE (OPT_MODE_HOST | OPT_MODE_FULL_SPEED)

// Let's make this large to be able to handle large HID Report descriptors for
// in particular some Force Feedback joysticks.
#define CFG_TUH_ENUMERATION_BUFSIZE 4096

#define CFG_TUH_HUB 4 // We will probably need some number of hubs.
#define CFG_TUH_HID 16 // Let's be generous with HID devices, up to 4 HID devices per hub.
#define CFG_TUH_DEVICE_MAX (CFG_TUH_HUB + CFG_TUH_HID) // add all the devices together.

#define CFG_TUH_HID_EPIN_BUFSIZE 64
#define CFG_TUH_HID_EPOUT_BUFSIZE 64

#endif // _TUSB_CONFIG_H_
