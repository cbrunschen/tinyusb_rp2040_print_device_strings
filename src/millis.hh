#ifndef __MILLIS_HH__
#define __MILLIS_HH__

#include <pico/time.h>

inline static uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }
inline static uint64_t micros() { return to_us_since_boot(get_absolute_time()); }

#endif // __MILLIS_HH__
