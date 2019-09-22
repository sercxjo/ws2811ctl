#pragma once
#include "private/network-config.h"

#include <espressif/esp_wifi.h>
#include <espressif/esp_softap.h>
#include <espressif/esp_system.h>
#include <espressif/esp_misc.h>
#include <dhcpserver.h>
#include <netif.h>
#include <string.h>

#ifdef  __cplusplus
extern "C" {
#endif
void wifi_init(void);

#ifdef  __cplusplus
void network_init(void)
{
    wifi_init();

    auto opmode= sdk_wifi_get_opmode();
    printf("WiFi mode %d\n", opmode);

    if (opmode == SOFTAP_MODE || opmode == STATIONAP_MODE) {
        uint8_t hwaddr[6];
        if (sdk_wifi_get_macaddr(SOFTAP_IF, hwaddr)) {
            printf("link/ether " MACSTR "\n", MAC2STR(hwaddr));
        }
        struct ip_info info;
        if (sdk_wifi_get_ip_info(SOFTAP_IF, &info)) {
            printf("AP inet " IPSTR " ", IP2STR(&info.ip));
            printf(" mask " IPSTR, IP2STR(&info.netmask));
            printf(" gw " IPSTR "\n", IP2STR(&info.gw));
        }

#if LWIP_IPV6
        struct netif *netif = sdk_system_get_netif(SOFTAP_IF);
        if (netif) {
            char buf[130];
            for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
                if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i))) {
                    const ip6_addr_t *addr6 = netif_ip6_addr(netif, i);
                    printf("inet6 %s\n", ip6addr_ntoa_r(addr6, buf, sizeof buf));
                }
            }
        }
#endif
    }
}

}
#endif
