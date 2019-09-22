#include "private/network-config.h"
#include "network.h"

#include <espressif/esp_wifi.h>
#include <espressif/esp_softap.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_system.h>
#include <espressif/esp_misc.h>
#include <libesphttpd/webpages-espfs.h>
#include <dhcpserver.h>
#include <netif.h>
#include <string.h>

void wifi_init() {
    struct ip_info ap_ip;
    sdk_wifi_set_opmode(STATION_MODE);
    switch(sdk_wifi_get_opmode()) {
        case STATIONAP_MODE:
        case SOFTAP_MODE:
            IP4_ADDR(&ap_ip.ip, 172, 16, 0, 1);
            IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
            IP4_ADDR(&ap_ip.netmask, 255, 255, 0, 0);
            sdk_wifi_set_ip_info(1, &ap_ip);

            struct sdk_softap_config ap_config = {
                .ssid = AP_SSID,
                .ssid_hidden = 0,
                .channel = 3,
                .ssid_len = strlen(AP_SSID),
                .authmode = AP_PASS && AP_PASS[0]? AUTH_WPA_WPA2_PSK: AUTH_OPEN,
                .password = AP_PASS,
                .max_connection = 3,
                .beacon_interval = 100,
            };
            sdk_wifi_softap_set_config(&ap_config);

            ip4_addr_t first_client_ip;
            IP4_ADDR(&first_client_ip, 172, 16, 0, 2);
            if (sdk_wifi_get_opmode() == SOFTAP_MODE) {
                dhcpserver_start(&first_client_ip, 4);
                dhcpserver_set_dns(&ap_ip.ip);
                dhcpserver_set_router(&ap_ip.ip);
                break;
            }
        case STATION_MODE: {
            /*struct sdk_station_config config = {
               .ssid = WIFI_SSID,
               .password = WIFI_PASS,
            };
            sdk_wifi_station_set_config(&config);*/
            //sdk_wifi_station_connect();
            break;
        }
    }

    auto fsr= espFsInit((void*)(_binary_build_web_espfs_bin_start));
    printf("espFsInit returned %d\n", fsr);

}
