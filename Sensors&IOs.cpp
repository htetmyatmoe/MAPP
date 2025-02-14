#undef __ARM_FP

#include "DHT11.h"
#include "keypad.h"
#include "lcd.h"
#include "mbed.h"
#include <cstdio>

#define WAIT_TIME_MS_0 500
#define WAIT_TIME_MS_1 1500
#define DHT11_PIN PD_2

#define PERIOD_WIDTH 20              // Servo motor period (20ms)
#define PULSE_WIDTH_100_DEGREE 2055  // Move to +100 degrees (CW)
#define PULSE_WIDTH_0_DEGREE 1500    // Middle position (0 degrees)
#define PULSE_WIDTH_N_100_DEGREE 945 // Move to -100 degrees (CCW)
#define WAIT_TIME_STEP 20            // Smaller wait time for smooth movement
#define STEP_SIZE 10                 // Gradual step size for smoother movement

AnalogIn waterLevelSensor(PA_0);
DHT11 dht11(PD_2);

DigitalOut redLED(PC_0);
DigitalOut greenLED(PC_1);
DigitalOut blueLED(PC_2);

PwmOut buzzer(PA_1);

PwmOut motor1(PA_7); // First Servo Motor (Clockwise)
PwmOut motor2(PA_6); // Second Servo Motor (Counterclockwise)

UnbufferedSerial serial(USBTX, USBRX, 9600);

const float max_voltage = 5.5;
const float max_distance_cm = 5.5;
const float yellow_level = 3.0;
const float red_level = 4.6;

unsigned int yellow_light = 0;
unsigned int green_light = 0;
unsigned int red_light = 0;

char Message6[] = "WaterLevel: ";
char WarningMsg[] = "Warning!";
char FloodMsg[] = "Flood Happening";
char keypadMsg[] = "Enter Password:";
char Message5[] = "Invalid key!";
char Message4[] = "1: Temperature";
char Message3[] = "2: Humidity";
char Message1[] = "Temperature: ";
char Message2[] = "Humidity: ";
char password[] = {'0', '0', '0', '0'};

char buffer1[20];
char buffer2[20];
char buffer3[20];

int i;
char Outchar;

int currentPulse1 = PULSE_WIDTH_0_DEGREE;
int currentPulse2 = PULSE_WIDTH_0_DEGREE;

volatile int buzzer_mode = 0;
volatile bool buzzer_state = false;
volatile bool buzzer_enabled = true;
volatile bool buzzer_suppressed = false;
volatile bool keypad_shown = false;

volatile int latestTemp = 0;
volatile int latestHumidity = 0;
volatile bool newDHTReading = false;
volatile bool displayWaterLevelState = true;

Ticker dhtTicker;
Ticker buzzerTicker;
Ticker displayTicker;

void triggerDHTUpdate() { newDHTReading = true; }

float getWaterLevel() {
  float adc = waterLevelSensor.read();
  float voltage = adc * max_voltage;
  float waterLevel = (voltage / max_voltage) * max_distance_cm;

  char serialBuffer[50];
  sprintf(serialBuffer, "ADC: %.2f, Voltage: %.2fV, Water Level: %.2f cm\n",
          adc, voltage, waterLevel);
  serial.write(serialBuffer, strlen(serialBuffer));

  return waterLevel;
}

void setRGB(float red, float green, float blue) {
  redLED = red;
  greenLED = green;
  blueLED = blue;
}

void blinkRGB(float red, float green, float blue, int blink_count,
              int blink_delay_ms) {
  for (int i = 0; i < blink_count; i++) {
    setRGB(red, green, blue);
    thread_sleep_for(blink_delay_ms);
    setRGB(0, 0, 0); // LED OFF
    thread_sleep_for(blink_delay_ms);
  }
}

void displayWaterLevel() {
  float waterlevel = getWaterLevel();
  sprintf(buffer3, "%f", waterlevel);
  lcd_write_cmd(0x01);
  lcd_write_cmd(0x80);
  for (int i = 0; i < 12; i++) {
    lcd_write_data(Message6[i]);
  }
  for (int i = 0; i < 4; i++) {
    lcd_write_data(buffer3[i]);
  }
  lcd_write_data('c');
  lcd_write_data('m');
}

void yellow_warning() {
  lcd_write_cmd(0x80);
  for (i = 0; i < 8; i++) {
    lcd_write_data(WarningMsg[i]);
  }
  thread_sleep_for(2000);
}

void red_warning() {

  lcd_write_cmd(0x80);
  for (i = 0; i < 15; i++) {
    lcd_write_data(FloodMsg[i]);
  }
  thread_sleep_for(2000);
}

void switchDisplay() {
  displayWaterLevelState =
      !displayWaterLevelState; // Toggle between water level and temp/humi
                               // display
}

void displayTempHumi() {
  int temperature = 0, humidity = 0;

  if (newDHTReading) { //  Only update values if new reading is available
    temperature = dht11.readTemperature();
    humidity = dht11.readHumidity();
    int result = dht11.readTemperatureHumidity(temperature, humidity);
    newDHTReading = false; // Reset flag after reading
  }

  sprintf(buffer1, "%d", temperature);
  sprintf(buffer2, "%d", humidity);

  lcd_write_cmd(0x01);

  lcd_write_cmd(0x80);
  for (i = 0; i < 13; i++) {
    Outchar = Message1[i];
    lcd_write_data(Outchar);
  }
  for (i = 0; i < 2; i++) {
    lcd_write_data(buffer1[i]);
  }
  lcd_write_data(223);
  lcd_write_data('C');

  lcd_write_cmd(0xC0);

  for (i = 0; i < 10; i++) {
    Outchar = Message2[i];
    lcd_write_data(Outchar);
  }
  for (i = 0; i < 2; i++) {
    lcd_write_data(buffer2[i]);
  }
  lcd_write_data('%');
}

/*void toggleBuzzer() {
    if (!buzzer_enabled || buzzer_mode == 0) {
        buzzer.write(0.0f); // Ensure buzzer is off if not needed
        return;
    }

    // Toggle buzzer state for continuous sound at red level
    buzzer_state = !buzzer_state;
    if (buzzer_state) {
        buzzer.period(1.0 / 800.0); // Set frequency (800 Hz)
        buzzer.write(0.5f);         // 50% duty cycle for continuous sound
    } else {
        buzzer.write(0.0f);
    }
} */

void toggleBuzzer() {
  if (!buzzer_enabled) {
    buzzer.write(0.0f);
    return;
  }
  if (buzzer_mode > 1) {
    buzzer_state = !buzzer_state;
    if (buzzer_state) {
      buzzer.period(buzzer_mode == 2 ? (1.0 / 400.0) : (1.0 / 800.0));
      buzzer.write(buzzer_mode == 2 ? 0.3f : 0.5f);
    } else {
      buzzer.write(0.0f);
    }
  } else {
    buzzer.write(0.0f);
  }
}

void MidTone_MidPitch(void) {
  float frequency = 450.0; // Low pitch frequency in Hz
  float period = 1.0 / frequency;

  // Set the PWM period and duty cycle

  buzzer.period(period); // Set the period of the waveform
  buzzer.write(0.3f);
  // Set duty cycle to 50% (on half the time)
  // Keep the tone on for 3 seconds
  thread_sleep_for(700);

  // Turn off the buzzer
  buzzer.write(0.0f);
  thread_sleep_for(700);
}

void LowTone_LowPitch(void) {
  float frequency = 400.0; // Low pitch frequency in Hz
  float period = 1.0 / frequency;

  // Set the PWM period and duty cycle

  buzzer.period(period); // Set the period of the waveform
  buzzer.write(0.04f);   // Set duty cycle to 50% (on half the time)

  // Keep the tone on for 3 seconds
  thread_sleep_for(2000);

  // Turn off the buzzer
  buzzer.write(0.0f);
  printf("Tone stopped\n");
}

void press_keypad() {
  if (buzzer_suppressed || keypad_shown)
    return;

  lcd_write_cmd(0x80);
  for (i = 0; i < 15; i++) {
    lcd_write_data(keypadMsg[i]);
  }

  char password[4] = {'\0', '\0', '\0', '\0'}; // Store the password
  int keyIndex = 0;
  while (keyIndex < 1) {

    lcd_write_cmd(0xC0); // Move cursor to line 2 position 1

    for (i = 0; i < 4; i++) {
      password[i] = getkey(); // wait and get an ascii key number when pressed
      lcd_write_data('*');    // display the PIN keyed in on LCD
      // lcd_write_data(key);          //comment the above line and de-comment
      // this line to display * instead of the actual PIN number keyed in
    }
    keyIndex++;
  }

  // Check if password is correct
  if (password[0] == '0' && password[1] == '0' && password[2] == '0' &&
      password[3] == '0') {
    buzzer_suppressed = true;
    buzzer_mode = 0;
    buzzer.write(0.0f);
    serial.write("Buzzer Stopped\n", 15);
    keypad_shown = true; // Prevent re-displaying the keypad
  } else {
    serial.write("Incorrect Password\n", 20);
    keypad_shown = false; // Allow retry
  }
}

void moveServos(int targetPulseMotor1, int targetPulseMotor2) {

  if (currentPulse1 < targetPulseMotor1) {
    for (int pulse1 = currentPulse1, pulse2 = currentPulse2;
         pulse1 <= targetPulseMotor1 && pulse2 >= targetPulseMotor2;
         pulse1 += STEP_SIZE, pulse2 -= STEP_SIZE) { // Opposite directions
      motor1.pulsewidth_us(pulse1);
      motor2.pulsewidth_us(pulse2);
      thread_sleep_for(WAIT_TIME_STEP); // Smooth transition
    }
  } else {
    for (int pulse1 = currentPulse1, pulse2 = currentPulse2;
         pulse1 >= targetPulseMotor1 && pulse2 <= targetPulseMotor2;
         pulse1 -= STEP_SIZE, pulse2 += STEP_SIZE) { // Opposite directions
      motor1.pulsewidth_us(pulse1);
      motor2.pulsewidth_us(pulse2);
      thread_sleep_for(WAIT_TIME_STEP); // Smooth transition
    }
  }
  // Update current positions
  currentPulse1 = targetPulseMotor1;
  currentPulse2 = targetPulseMotor2;
}

int main() {
  lcd_init();
  lcd_Clear();
  buzzerTicker.attach(&toggleBuzzer, 100ms);
  dhtTicker.attach(&triggerDHTUpdate, 5s);
  displayTicker.attach(&switchDisplay, 5s);

  bool first_red_transition = true; // Track first transition into red

  bool was_red_level = false;
  bool was_yellow_level = false;
  bool was_green_level = false;

  motor1.period_ms(PERIOD_WIDTH); // Set PWM period to 20ms
  motor2.period_ms(PERIOD_WIDTH);

  motor1.pulsewidth_us(PULSE_WIDTH_0_DEGREE); // Start both at 0 degrees
  motor2.pulsewidth_us(PULSE_WIDTH_0_DEGREE);

  printf("Starting at 0 degrees\n");
  thread_sleep_for(500); // Wait for servo to stabilize

  while (1) {
    float waterLevel = getWaterLevel();

    if (waterLevel >= red_level) {
      blinkRGB(1, 0, 0, 1, 250);
      serial.write("RED LED on\n", 12);
      displayWaterLevel();
      lcd_write_cmd(0x01);
      setRGB(1, 0, 0);
      red_warning();

      if (first_red_transition) {
        buzzer_suppressed = false;
        keypad_shown = false;
        buzzer_mode = 3;              // Trigger buzzer sound
        first_red_transition = false; // Prevent future re-triggering in red
      }

      was_red_level = true;    // Now in red level
      was_green_level = false; // Prevents retriggering
      lcd_write_cmd(0x01);

      // Keep keypad active until correct password is entered
      while (!buzzer_suppressed) {
        press_keypad();
      }

    }

    else if (waterLevel >= yellow_level) {
      blinkRGB(1, 1, 0, 1, 250);
      serial.write("Yellow LED on\n", 15);
      displayWaterLevel();
      lcd_write_cmd(0x01);
      setRGB(1, 1, 0);
      yellow_warning();
      while (green_light > 0) {
        green_light--;
      }

      printf("PA_7 CW to +100°, PA_6 CCW to -100°\n");
      moveServos(PULSE_WIDTH_100_DEGREE, PULSE_WIDTH_N_100_DEGREE);

      /*if (!was_yellow_level) {
        buzzer_suppressed = false;
        was_yellow_level = true;
      } */

      was_red_level = false;
      was_green_level = false;

      if (!was_yellow_level) {
        buzzer_suppressed = false;
        was_yellow_level = true;
      }

      while (yellow_light < 1) {
          for(int i = 0 ; i <5 ; i++){
        MidTone_MidPitch();
        yellow_light++;
      }
      }
    }

    else {
      setRGB(0, 1, 0);
      serial.write("Green LED on\n", 12);

      // Reset yellow and red level flags so next red transition triggers buzzer
      was_red_level = false;
      was_yellow_level = false;
      was_green_level = true; // Now in green level

      // If first time reaching green after a full cycle, reset
      // first_red_transition
      first_red_transition = true;

      while (yellow_light > 0) {
        yellow_light--;
      }

      while (green_light < 1) {
        LowTone_LowPitch(); // Green should only beep once
        green_light++;
      }

      keypad_shown = false;

      printf("PA_7 CCW to -100°, PA_6 CW to +100°\n");
      moveServos(PULSE_WIDTH_N_100_DEGREE, PULSE_WIDTH_100_DEGREE);

      if (displayWaterLevelState) {
        displayWaterLevel();
        thread_sleep_for(3000);
      } else {
        displayTempHumi();
        thread_sleep_for(3000);
      }
    }
  }
}

