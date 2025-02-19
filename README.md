# Smart Gate System for Flooding
This project is an IoT-based flood monitoring and alert system using STM32 and Mbed OS. It detects water levels, displays real-time data on an LCD, and activates a servo-based gate system when flooding occurs.


## Features
- Water level detection using a water level sensor
- Real-time display of temperature and humidity on an LCD
- Automatic gate control with two servo motors
- RGB light according to the flooding condition 
- Buzzer tone according to the flooding condition
- KeyPad password to stop the buzzer at red water level
- Remote monitoring via WiFi communication

## Hardware Requirements
- STM32 Nucleo-F103RB
- Water level sensor llc4965
- DHT11 Temperature and Humidity Sensor
- Servo Motor (SG90)
- 16x2 LCD Display with I2C Module
- WiFi Module (ESP8266)
- RGB 
- Buzzer

## Software Requirements
- Mbed Studio or Keil uVision
- Mbed OS
- CMake (for build configuration)


## Installation & Setup
Clone the repository:
```sh
git clone https://github.com/htetmyatmoe/MAPP.git
cd MAPP

mbed config root .
mbed deploy

mbed compile -t GCC_ARM -m NUCLEO_F103RB


---

### **5. Usage**
- Provide instructions on how to use the project.

**Example:**
```md
## Usage
1. Power on the system.
2. Monitor real-time water levels and temperature on the LCD.
3. If flooding is detected, the gate will automatically open.
4. RGB and Buzzer trigger according to water level
5. Data is sent to a remote monitoring system via WiFi.

## Project Structure
MAPP/
│-- src/
│   ├── dht11_utilities.cpp  # Reads temperature & humidity from the DHT11 sensor
│   ├── keypad_utilities.cpp  # Handles user input via a capacitive touch keypad
│   ├── lcd_utilities.cpp  # Controls the 16x2 LCD display using the I2C module
│   ├── wifi.cpp  # Manages WiFi communication for remote monitoring
│   ├── Sensors_IOs.cpp  # Integrates sensor inputs (ultrasonic, DHT11) and handles outputs
│-- include/
│   ├── dht11_utilities.h  # Header file for DHT11 sensor functions
│   ├── lcd_utilities.h  # Header file for LCD display functions
│   ├── keypad_utilities.h  # Header file for keypad input functions
│   ├── wifi.h  # Header file for WiFi communication functions
│-- CMakeLists.txt  # CMake build configuration for compiling the project
│-- mbed_app.json  # Configuration file for Mbed OS settings and target board
│-- README.md  # Project documentation and setup instructions
│-- LICENSE  # Apache-2.0 License defining usage terms
│-- .github/ISSUE_TEMPLATE/  # GitHub issue reporting templates



## Contact
- Developer: Htet Myat Moe, Ooi Zi Xian, Ravinash S Kunalan
- Email: htetmyatmoe2311@gmail.com
- GitHub: [htetmyatmoe](https://github.com/htetmyatmoe)

