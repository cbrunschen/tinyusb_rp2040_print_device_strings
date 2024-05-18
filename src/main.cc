#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tusb.h>

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include "tusb_option.h"
#include "millis.hh"

#include <deque>
#include <set>
#include <string>

#define IDX_NONE 0
#define IDX_MANUFACTURER 1
#define IDX_PRODUCT 2
#define IDX_SERIAL 3

const char *string_name[] {
  "N/A",
  "Manufacturer",
  "Product",
  "Serial"
};

struct string_to_get {
  uint8_t dev_addr;
  uint8_t idx;
};

std::set<uint8_t> devices;

std::deque<string_to_get> strings_to_get;

bool getting_string = false;

bool get_string(uint8_t dev_addr, uint8_t idx);

#define MIN_QUIESCENT_MILLIS 5000

uint32_t last_activity = 0;

int main(void) {  
  stdio_init_all();
  
  printf("*** Fetch Device Strings ***\r\n");
      
  printf("Initializing USB Host.\r\n");
  tuh_init(BOARD_TUH_RHPORT);
  printf("Started USB Host.\r\n");
  
  last_activity = millis();

  while (true) {
    tuh_task(); // tinyusb host task
    
    if (!getting_string && !strings_to_get.empty() && (millis() - last_activity) > MIN_QUIESCENT_MILLIS) {
      auto string_to_get = strings_to_get.front();
      printf("@ %ld: Fetching [%d] %s string\r\n", millis(), string_to_get.dev_addr, string_name[string_to_get.idx]);
      if (!(getting_string = get_string(string_to_get.dev_addr, string_to_get.idx))) {
        printf("  returns false.\r\n");
      }      
      strings_to_get.pop_front();
    }
    
    // Echo any input back to the output
    {
      int c = getchar_timeout_us(0);
      if (0 <= c && c < 256) {
        putchar_raw((char) (c & 0xff));
      }
    }
  }

  return 0;
}

uint8_t _string_buffer[256];
void _have_string(tuh_xfer_t* xfer) {
  uint8_t dev_addr = xfer->daddr;
  uint8_t idx = (uint8_t)xfer->user_data;
  
  // Extract the length of the string
  uint8_t len_bytes = _string_buffer[0];
  uint8_t n_bytes = len_bytes - 2;
  const uint8_t *rp = _string_buffer + 2;
  
  printf("@ %ld: Have [%d] %s string, %d bytes = [", millis(), dev_addr, string_name[idx], n_bytes);
  for (int i = 0; i < n_bytes; i++) {
    if (i != 0) printf(" ");
    printf("%02x", rp[i]);
  }
  printf("] = '");
  for (int i = 0; i < n_bytes; i+=2) {
    uint8_t c = rp[i];
    if (32 <= c && c <= 127) printf("%c", (char)c);
    else printf(".");
  }
  printf("'\r\n");
  
  getting_string = false;
}

bool get_string(uint8_t dev_addr, uint8_t idx) {
  memset(_string_buffer, 0, sizeof(_string_buffer));
  if (idx == IDX_MANUFACTURER) {
    return tuh_descriptor_get_manufacturer_string(dev_addr, 0x0409, _string_buffer, sizeof(_string_buffer), _have_string, (uintptr_t)idx);
  } else if (idx == IDX_PRODUCT) {
    return tuh_descriptor_get_product_string(dev_addr, 0x0409, _string_buffer, sizeof(_string_buffer), _have_string, (uintptr_t)idx);
  } else if (idx == IDX_SERIAL) {
    return tuh_descriptor_get_serial_string(dev_addr, 0x0409, _string_buffer, sizeof(_string_buffer), _have_string, (uintptr_t)idx);
  }
  return false;
}

//--------------------------------------------------------------------+
// Host HID
//--------------------------------------------------------------------+

void fetch_all_strings() {
  for (auto device : devices) {
    strings_to_get.emplace_back(device, IDX_MANUFACTURER);
    strings_to_get.emplace_back(device, IDX_PRODUCT);
    strings_to_get.emplace_back(device, IDX_SERIAL);
  }
}

// Invoked when device with hid instance is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = nullptr, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  last_activity = millis();
    
  printf("@ %ld: [%d.%d] [%04x:%04x] connected, descriptor has %d bytes\r\n", millis(), dev_addr, instance, vid, pid, desc_len);
  (void) desc_report;
  devices.insert(dev_addr);
  fetch_all_strings();
  
}

// Invoked when device with hid instance is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  last_activity = millis();

  printf("@ %ld: [%d.%d] disconnected\r\n", millis(), dev_addr, instance);
  devices.erase(dev_addr);
  fetch_all_strings();
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
  (void) dev_addr;
  (void) instance;
  (void) report;
  (void) len;
  last_activity = millis();
}

