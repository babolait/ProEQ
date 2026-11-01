#ifndef IS_FUNCTIONS_H
#define IS_FUNCTIONS_H

#include <Arduino.h>
#include <Wire.h>

// I2C IS defines
#define IS_1_ADDR 0x20
#define IS_2_ADDR 0x23
#define DOT_DIM 0x11
#define MATIAS_DIM 0xFF
#define PWM_REG 0x00
#define SCALLING_REG 0x01
#define FUNCTION_REG 0x02

// Constants
#define VU_MAX_VALUE 4080
#define ENC_MAX_VALUE 127
#define ENC_RING_MAX_VALUE 3570  // 14 * 255
#define NUM_SWITCHES 15
#define VU_LEDS_PER_SIDE 16

#define NUM_ENCODERS 18

// VU Meter sides
enum VUSide
{
  VU_LEFT = 0, VU_RIGHT = 1
};

// Encoder ring mapping structure
struct EncoderRing
{
  uint8_t startLed;  // First LED of the ring
  uint8_t driver;    // 0 for IS1, 1 for IS2
};

// Switch LED mapping (switch number -> {led1, led2})
struct SwitchLEDs
{
  uint8_t led1;
  uint8_t led2;
};

// Function prototypes
void initIS(byte address);
void initScalling(byte address);
void selectPage(uint8_t address, uint8_t page);
void setValue(byte address, byte led, byte value);
void setPWM(byte address, byte led, byte value);
void setVumeter(VUSide side, uint16_t value);
void setEncoderRing(uint8_t encNumber, uint8_t encValue);
void setSwitch(uint8_t switchNumber, bool switchValue);
void updateEncoderRing(uint8_t encNumber, uint16_t ringValue);

#endif
