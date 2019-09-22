#include "network.h"

#include <string.h>
#include <stdio.h>

#include <FreeRTOS.h>
#include <esp/uart.h>
#include <espressif/esp_common.h> // sdk_os_delay_us
#include <Ws2811.h>
#include <task.h>

#include <libesphttpd/espfs.h>

#include "webserver.h"

#define STRIP_PIN 14 // GPIO14
#define LEDS 150

Ws2811 strip(STRIP_PIN);
extern "C" void strip_drv(void *pvParameters)
{
    strip.zone.emplace_back( std::make_shared<PlazmaZone>(0, LEDS) );
    for (;;) {
        vTaskDelay(configTICK_RATE_HZ/25);
        strip.send_strip();
    }
}

// Main routine. Initialize all.
extern "C" void user_init(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    uart_set_baud(0, 115200);

    network_init();
    xTaskCreate(&strip_drv, "strip-drv", 256, NULL, 20, NULL);
    http_init();
}

// Implement necessary for C++ programs but absent in library functions
extern "C" void __cxa_pure_virtual() {
    abort();
}

namespace std {
  void __throw_bad_alloc() {
    abort();
  }
  void __throw_length_error(char const*) {
    abort();
  }
}
