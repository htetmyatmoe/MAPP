#include "wifi.h"
#include <stdio.h>
#include <string.h>

// Define UART pins (Change if needed)
#define WIFI_TX PC_10
#define WIFI_RX PC_11

// Create a BufferedSerial object for ESP8266 communication
BufferedSerial wifi_serial(WIFI_TX, WIFI_RX, 115200);

char wifi_bufTx[256] = {0};
char wifi_bufRx[256] = {0};

volatile bool floodAlertSent = false;  // Flag to track if the alert was sent

void setupWiFi() {
    printf("Initializing WiFi...\n");

    // Send AT command to check if ESP8266 is responding
    sprintf(wifi_bufTx, "AT\r\n");
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(1000);

    memset(wifi_bufRx, 0, sizeof(wifi_bufRx)); // Clear buffer
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("ESP01 Response: %s\n", wifi_bufRx);

    // Set WiFi mode (Station Mode)
    sprintf(wifi_bufTx, "AT+CWMODE=1\r\n");
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(1000);

    // Connect to WiFi
    char ssid[] = "opshidoinurarea";        
    char password[] = "abcde12345"; 
    snprintf(wifi_bufTx, sizeof(wifi_bufTx), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(5000);

    memset(wifi_bufRx, 0, sizeof(wifi_bufRx)); // Clear buffer
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("WiFi Connection Status: %s\n", wifi_bufRx);

    // ** Fetch and Print IP Address **
    printf("Fetching IP Address...\n");
    sprintf(wifi_bufTx, "AT+CIFSR\r\n");  // Get IP address
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(2000);  // Wait for ESP8266 to respond

    // Read IP address properly
    memset(wifi_bufRx, 0, sizeof(wifi_bufRx)); // Clear buffer
    int bytesRead = wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    
    if (bytesRead > 0) {
        wifi_bufRx[bytesRead] = '\0'; // Null terminate the string
        printf("ESP8266 IP Address: %s\n", wifi_bufRx);
    } else {
        printf("Failed to get IP address. Check WiFi connection.\n");
    }

    // Enable multiple connections
    sprintf(wifi_bufTx, "AT+CIPMUX=1\r\n");
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(1000);

    // Start TCP server on **PORT 3333**
    sprintf(wifi_bufTx, "AT+CIPSERVER=1,3333\r\n");
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(5000);

    memset(wifi_bufRx, 0, sizeof(wifi_bufRx)); // Clear buffer
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("Server Status: %s\n", wifi_bufRx);

    printf("WiFi Setup Completed!\n");
}





void sendFloodAlert() {
    printf("Checking Server Connection...\n");

    // Request connection status
    sprintf(wifi_bufTx, "AT+CIPSTATUS\r\n");
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(500);

    // Read response
    memset(wifi_bufRx, 0, sizeof(wifi_bufRx));
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("ESP8266 Status Response: %s\n", wifi_bufRx);

    // Check if no client is connected
    if (strstr(wifi_bufRx, "STATUS:4") != NULL) {
        printf(" No TCP client connected. Flood Alert NOT sent.\n");
        return;  // Exit the function without sending the alert
    }

    printf("TCP Client Connected. Sending Flood Alert...\n");

    // Define the alert message
    const char *alertMessage = "Flood Alert! Water Level Critical!\r\n";

    // Calculate actual message length
    int messageLength = strlen(alertMessage);

    // Send AT+CIPSEND with the correct length
    snprintf(wifi_bufTx, sizeof(wifi_bufTx), "AT+CIPSEND=0,%d\r\n", messageLength);
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(500);

    // Wait for confirmation before sending the message
    memset(wifi_bufRx, 0, sizeof(wifi_bufRx));
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("ESP Response: %s\n", wifi_bufRx);

    // Send the actual alert message
    wifi_serial.write(alertMessage, messageLength);
    thread_sleep_for(1000);

    memset(wifi_bufRx, 0, sizeof(wifi_bufRx));
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("Reply from ESP01: %s\n", wifi_bufRx);

    printf(" Flood Alert Sent Successfully!\n");
}

void sendBarrierStatus(const char *statusMessage) {
    printf("Checking Server Connection for Barrier Status...\n");

    // Request connection status
    snprintf(wifi_bufTx, sizeof(wifi_bufTx), "AT+CIPSTATUS\r\n");
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(500);

    // Read response
    memset(wifi_bufRx, 0, sizeof(wifi_bufRx));
    int bytesRead = wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx) - 1);
    wifi_bufRx[bytesRead] = '\0';

    printf("ESP8266 Status Response: %s\n", wifi_bufRx);

    // Check if no client is connected
    if (strstr(wifi_bufRx, "STATUS:4") != NULL) {
        printf(" No TCP client connected. Barrier Status NOT sent.\n");
        return;
    }

    printf(" TCP Client Connected. Sending Barrier Status...\n");

    // Calculate actual message length
    int messageLength = strlen(statusMessage);

    // Send AT+CIPSEND with the correct length
    snprintf(wifi_bufTx, sizeof(wifi_bufTx), "AT+CIPSEND=0,%d\r\n", messageLength);
    wifi_serial.write(wifi_bufTx, strlen(wifi_bufTx));
    thread_sleep_for(500);

    // Wait for confirmation before sending the message
    memset(wifi_bufRx, 0, sizeof(wifi_bufRx));
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("ESP Response: %s\n", wifi_bufRx);

    // Send the actual barrier status message
    wifi_serial.write(statusMessage, messageLength);
    thread_sleep_for(1000);

    memset(wifi_bufRx, 0, sizeof(wifi_bufRx));
    wifi_serial.read(wifi_bufRx, sizeof(wifi_bufRx));
    printf("Reply from ESP01: %s\n", wifi_bufRx);

    printf("Barrier Status Sent Successfully!\n");
}
