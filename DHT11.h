#ifndef DHT11_h
#define DHT11_h
#undef __ARM_FP
#include <chrono>
using namespace std::chrono;
#include "mbed.h"
#define byte unsigned char
class DHT11
{
public:
/**
* Constructor
* Initializes the data pin to be used for communication with the DHT11 sensor.
*
* @param pin: Pin Name on the STM32 NUCLEO-F103RB board to which the DHT11 sensor is
connected.
*/
DHT11(PinName pin);
/**
* Sets the delay between consecutive sensor readings.
* If this method is not called, a default delay of 500 milliseconds is used.
*
* @param delay: Delay duration in milliseconds between sensor readings.
*/
void setDelay(unsigned long delay);
/**
* Reads and returns the humidity from the DHT11 sensor.
*
* @return: Humidity value in percentage. Returns DHT11_ERROR_TIMEOUT if reading times
out.
* Returns DHT11_ERROR_CHECKSUM if checksum validation fails.
*/
int readHumidity();
/**
* Reads and returns the temperature from the DHT11 sensor.
*
* @return: Temperature value in Celsius. Returns DHT11_ERROR_TIMEOUT if reading times
out.
* Returns DHT11_ERROR_CHECKSUM if checksum validation fails.
*/
int readTemperature();
/**
* Reads and returns the temperature and humidity from the DHT11 sensor.
*
* @param temperature: Reference to a variable where the temperature value will be
stored.
* @param humidity: Reference to a variable where the humidity value will be stored.
16
Official (Closed), Non‐Sensitive
* @return: true if the reading is successful, false if it fails due to timeout or
checksum error.
*/
int readTemperatureHumidity(int &temperature, int &humidity);
// Constants to represent error codes.
static const int ERROR_CHECKSUM = 254; // Error code indicating checksum mismatch.
static const int ERROR_TIMEOUT = 253; // Error code indicating a timeout occurred

static const int TIMEOUT_DURATION = 1000; // Duration (in milliseconds) to wait before

/**
* Returns a human-readable error message based on the provided error code.
*
* @param errorCode: The error code for which the message is required.
* @return: A string describing the error.
*/
char* getErrorString(int errorCode);
private:
PinName _pin; // Pin Name used for communication with the DHT11 sensor.
unsigned long _delayMS = 500; // Default delay in milliseconds between sensor readings.
Timer t;
/**
* Private method to read raw data from the DHT11 sensor.
* This method encapsulates the communication with the sensor and data reading process,
* and is utilized by public methods to get temperature and humidity data.
*
* @param data: Array to store the raw data read from the sensor.
* @return: An integer representing the status of the read operation.
* Returns 0 if the reading is successful, DHT11::ERROR_TIMEOUT if a timeout
occurs,
* or DHT11::ERROR_CHECKSUM if a checksum error occurs.
*/
int readRawData(byte data[5]);
};
#endif
