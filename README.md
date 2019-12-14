# Control ws2811-based pixel led strip with ESP8266 using OpenRTOS
## Features
* Web control interface
* Simple API with binary form of data.
* Client-side data binding using javascript.
* Server part written on C/C++.

## Compile & Install
* Clone esp-open-rtos and install all its prerequisites as described on https://github.com/SuperHouse/esp-open-rtos
* Clone this repository into examlpes using option --recurse-submodules 
* Create directory 'private' and file 'private/network-config.h' in the source tree of this project. The file must contain macro defines:
```c
#define WIFI_SSID "youWifiSsid"
#define WIFI_PASS "youWifiPassword"
#define AP_SSID "TheEspAPSsid"
#define AP_PASS "EspAPPass"
```
* Build the project and flash it with usb/serial port
```bash
make flash
```
* Assign constatnt IP to you device on the DHCP server and connect with browser using http.
