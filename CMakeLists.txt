cmake_minimum_required(VERSION 3.13)

# add our own board definition headers, for the adafruit feather rp2040 with usb host
set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/boards")

# also use our implementation of TinyUSB
set(PICO_TINYUSB_PATH "${CMAKE_CURRENT_LIST_DIR}/../tinyusb")

include (pico_sdk_import.cmake)

project(stick-mixer C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(PICO_CXX_ENABLE_RTTI 1)

pico_sdk_init()

if (PICO_PLATFORM STREQUAL "host")
  # Build for HOST - i.e., tests etc
  message("Building Host Tests &c")

else()

  message("Building Device Firmware &c")
  
  set(target_name print_device_strings)

  add_executable(${target_name})

  target_sources(${target_name} PRIVATE
   src/main.cc
  )

   # print memory usage, enable all warnings
  target_link_options(${target_name} PRIVATE -Xlinker --print-memory-usage)
  target_compile_options(${target_name} PRIVATE -Wall -Wextra)

  # compile definitions:
  target_compile_definitions(${target_name} PRIVATE
    CFG_TUSB_CONFIG_FILE="tusb_config.h"
    CFG_TUSB_DEBUG=2
  )

  # needed so tinyusb can find tusb_config.h
  target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

  target_link_libraries(${target_name} PRIVATE
    # pico high-level
    pico_stdlib
    pico_multicore
    pico_flash

    # pico hardware
    hardware_exception
    hardware_flash
    hardware_pwm
    hardware_pio

    # tinyusb
    tinyusb_host
    tinyusb_bsp
  )
  
  pico_add_extra_outputs(${target_name})
  
  # STDIO through UART on pins 0 and 1
  pico_enable_stdio_usb(${target_name} 0)
  pico_enable_stdio_uart(${target_name} 1)
  target_compile_definitions(${target_name} PRIVATE
  	PICO_DEFAULT_UART_TX_PIN=0
  	PICO_DEFAULT_UART_RX_PIN=1
  )

endif() # PICO_PLATFORM != 'host'
