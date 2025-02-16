#ifndef WIFI_H
#define WIFI_H
#undef __ARM_FP
#include "mbed.h"

extern BufferedSerial serial_port;  // Declare the serial port globally

void setupWiFi();           // Function to configure the WiFi module
void sendFloodAlert();      // Function to send flood alert
void sendBarrierStatus(const char *statusMessage);

#endif // WIFI_H
