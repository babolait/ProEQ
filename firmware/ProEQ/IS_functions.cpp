#include "IS_functions.h"

// External declarations
extern const uint8_t mechSwitchAddress[33];
extern EncoderRing encoderRings[NUM_ENCODERS];
extern SwitchLEDs switchLEDs[NUM_SWITCHES];
extern const uint8_t encoderToRingMap[NUM_ENCODERS];

const uint8_t mechSwitchAddress[33] = {0x10, 0x11, 0x12, 0x22, 0x23, 0x24,
                                       0x34, 0x35, 0x36, 0x46,
                                       0x47, 0x48, 0x58, 0x59, 0x5A, 0x6A,
                                       0x6B, 0x6C, 0x7C, 0x7D, 0x7E, 0x8E, 0x8F, 0x90, 0xA0, 0xA1, 0xA2, 0xB2, 0xB3,
                                       0xB4, 0xC4, 0xC5, 0xC6
                                      };

// Encoder number to ring number mapping (encoder number -> ring number)
// Encoder 1 has no ring (index 0)
const uint8_t encoderToRingMap[NUM_ENCODERS] =
{
  0,   // Encoder 1 -> no ring
  1,   // Encoder 2 -> ring 1
  2,   // Encoder 3 -> ring 2
  5,   // Encoder 4 -> ring 5
  4,   // Encoder 5 -> ring 4
  3,   // Encoder 6 -> ring 3
  6,   // Encoder 7 -> ring 6
  8,   // Encoder 8 -> ring 8
  7,   // Encoder 9 -> ring 7
  11,  // Encoder 10 -> ring 11
  10,  // Encoder 11 -> ring 10
  9,   // Encoder 12 -> ring 9
  12,  // Encoder 13 -> ring 12 (split between drivers)
  14,  // Encoder 14 -> ring 14
  13,  // Encoder 15 -> ring 13
  18,  // Encoder 16 -> ring 18
  16,  // Encoder 17 -> ring 16
  15   // Encoder 18 -> ring 15
};
// Ring mapping for IS dirver and led number
EncoderRing encoderRings[NUM_ENCODERS] =
{
  {0, 0}, // Ring 0  -> for encoder 1, no rings attached
  {33, 0}, // Ring 1
  {47, 0}, // Ring 2
  {61, 0},  // Ring 3
  {75, 0},  // Ring 4
  {89, 0},  // Ring 5
  {103, 0},  // Ring 6
  {117, 0},  // Ring 7
  {131, 0},  // Ring 8
  {145, 0},  // Ring 9
  {159, 0},  // Ring 10
  {173, 0},  // Ring 11
  {187, 0},  // Ring 12
  {3, 1},   // Ring 13
  {17, 1},  // Ring 14
  {31, 1},  // Ring 15
  {45, 1},  // Ring 16
  {59, 1}   // Ring 17
};

SwitchLEDs switchLEDs[NUM_SWITCHES] =
{
  {0x11, 0xC0}, // Switch 1: SW1/CS17 & SW11/CS16
  {0x10, 0x24}, // Switch 2: SW1/CS16 & SW2/CS16
  {0x23, 0x35}, // Switch 3: SW2/CS17 & SW3/CS17
  {0x34, 0x22}, // Switch 4: SW4/CS16 & SW3/CS16
  {0x47, 0x46}, // Switch 5: SW5/CS17 & SW4/CS17
  {0x46, 0x58}, // Switch 6: SW5/CS16 & SW6/CS16
  {0x6B, 0x6A}, // Switch 7: SW7/CS17 & SW6/CS17
  {0x6A, 0x7C}, // Switch 8: SW7/CS16 & SW8/CS16
  {0x8F, 0x8E}, // Switch 9: SW9/CS17 & SW8/CS17
  {0x8E, 0xA0}, // Switch 10: SW10/CS16 & SW9/CS16
  {0xA1, 0xB3}, // Switch 11: SW10/CS17 & SW11/CS17
  {0x12, 0x36}, // Switch 12: SW1/CS18 & SW2/CS18
  {0x48, 0x5A}, // Switch 13: SW3/CS18 & SW4/CS18
  {0x6C, 0x7E}, // Switch 14: SW5/CS18 & SW6/CS18
  {0x90, 0xA2}  // Switch 15: SW7/CS18 & SW8/CS18
};

void initIS(byte address)
{
  selectPage(address, FUNCTION_REG);

  Wire.beginTransmission(address); // transmit to device 0x20
  Wire.write(0x01);        // GCC
  Wire.write(0xFF);        // set I max
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(address); // transmit to device 0x20
  Wire.write(0x02);        // pull up
  Wire.write(0x00);        // 8k pull down on SWx when off
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(address); // transmit to device 0x20
  Wire.write(0x00);        // configuration register
  Wire.write(0x01);        // dtasheet page 14
  Wire.endTransmission();    // stop transmitting

  selectPage(address, PWM_REG);

  for (int i = 1; i < 0xC7; i++)
  {
    setValue(address, i, 0x00);
  }

  selectPage(address, SCALLING_REG);

  for (int i = 1; i < 0xC7; i++)
  {
    setValue(address, i, 0xFF);
  }
}

void initScalling(byte address)
{
  selectPage(address, SCALLING_REG);

  // Dimming of dot led
  for (int i = 1; i < 0xC7; i++)
  {
    setValue(address, i, DOT_DIM);
  }
  // Dimming of matias (starting from SW_1 / CS_16, only for IS 2)
  if (address == IS_2_ADDR)
  {
    for (int i = 0; i < 33; i++)
    {
      setValue(address, mechSwitchAddress[i], MATIAS_DIM); //scalling 0xff for mach switchs
    }
  }
  selectPage(address, PWM_REG); // Now back to PWM for main functions
}

void selectPage(uint8_t address, uint8_t page)
{
  Wire.beginTransmission(address); // transmit to device 0x20
  Wire.write(0xFE);        // enable master pointer
  Wire.write(0xC5);        // code to enable
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(address); // transmit to device 0x20
  Wire.write(0xFD);        // control master register pointer
  Wire.write(page);        // scaling
  Wire.endTransmission();    // stop transmitting
}

// Set value for a specific LED
void setValue(byte address, byte led, byte value)
{
  // Write to the register
  Wire.beginTransmission(address);
  Wire.write(led);
  Wire.write(value);
  Wire.endTransmission();
}


// Set VU meter value
void setVumeter(VUSide side, uint16_t value)
{
  // Clamp value
  if (value > VU_MAX_VALUE) value = VU_MAX_VALUE;

  // Calculate how many LEDs should be fully on
  uint8_t fullLeds = value / 255;

  // Calculate PWM for the partially lit LED
  uint8_t partialPWM = value % 255;

  // VU meter LED mapping
  // Left: D1, D3, D5, ..., D31 (odd numbers starting from 1)
  // Right: D2, D4, D6, ..., D32 (even numbers starting from 2)

  for (uint8_t i = 0; i < VU_LEDS_PER_SIDE; i++)
  {
    uint8_t ledNumber;
    uint8_t pwmValue;

    // Calculate LED number based on side
    if (side == VU_LEFT)
    {
      ledNumber = 1 + (i * 2);  // 1, 3, 5, ..., 31
    }
    else
    {
      ledNumber = 2 + (i * 2);  // 2, 4, 6, ..., 32
    }

    // Calculate PWM value
    if (i < fullLeds)
    {
      pwmValue = 255;
    }
    else if (i == fullLeds)
    {
      pwmValue = partialPWM;
    }
    else
    {
      pwmValue = 0;
    }

    // Set PWM value
    // VU meter LEDs are on IS1 (D1-D32)
    setValue(IS_1_ADDR, ledNumber, pwmValue);
  }
}

// Set encoder ring based on encoder value (0-127)
void setEncoderRing(uint8_t encNumber, uint8_t encValue)
{
  /*
    selectPage(IS_2_ADDR, PWM_REG);
    setValue(IS_2_ADDR, 0x3b, 0xff);
    setValue(IS_2_ADDR, 0x3C, 0xff);*/

  // Skip encoder 1 (no ring)
  if (encNumber == 0 || encNumber >= NUM_ENCODERS) return;

  // Clamp value
  if (encValue > ENC_MAX_VALUE) encValue = ENC_MAX_VALUE;

  // Get the actual ring number from the encoder number
  uint8_t ringNumber = encoderToRingMap[encNumber];
  
  // Convert encoder value to ring value (0-3570)
  uint16_t ringValue = map(encValue, 0, ENC_MAX_VALUE, 0, ENC_RING_MAX_VALUE);

  Serial.println("INSIDE setEncoderRing");
  Serial.print("encNumber : ");
  Serial.println(encNumber);

  // Update the ring
  updateEncoderRing(encNumber, ringValue);
}

// Update encoder ring with specific ring value (0-3570)
void updateEncoderRing(uint8_t ringNumber, uint16_t ringValue)
{
  // Get the ring configuration
  EncoderRing ring = encoderRings[ringNumber];

  Serial.println("INSIDE updateEncoderRing");
  Serial.print("ringNumber : ");
  Serial.println(ringNumber);
  Serial.print("ringValue : ");
  Serial.println(ringValue);
  Serial.println();


  // Clamp value
  if (ringValue > ENC_RING_MAX_VALUE) ringValue = ENC_RING_MAX_VALUE;

  // Calculate how many LEDs should be fully on
  uint8_t fullLeds = ringValue / 255;

  // Calculate PWM for the partially lit LED
  uint8_t partialPWM = ringValue % 255;

  // Special handling for ring 12 (split between drivers)
  if (ringNumber == 12)
  {
    // First 12 LEDs on IS1
    for (uint8_t i = 0; i < 12; i++)
    {
      uint8_t pwmValue;

      if (i < fullLeds)
      {
        pwmValue = 255;
      }
      else if (i == fullLeds)
      {
        pwmValue = partialPWM;
      }
      else
      {
        pwmValue = 0;
      }

      uint8_t ledAddress = ring.startLed + i;
      selectPage(IS_1_ADDR, PWM_REG);
      setValue(IS_1_ADDR, ledAddress, pwmValue);
    }
    // Last 2 LEDs on IS2 (starting at LED 1 on IS2)
    for (uint8_t i = 12; i < 14; i++)
    {
      uint8_t pwmValue;

      if (i < fullLeds)
      {
        pwmValue = 255;
      }
      else if (i == fullLeds)
      {
        pwmValue = partialPWM;
      }
      else
      {
        pwmValue = 0;
      }

      // For IS2, LEDs start at 1 for ring 12 continuation
      uint8_t ledAddress = (i - 12) + 1; // LED 1 and 2 on IS2
      selectPage(IS_2_ADDR, PWM_REG);
      setValue(IS_2_ADDR, ledAddress, pwmValue);
    }
  }
  else
  {
    //selectPage(IS_2_ADDR, PWM_REG);

    // Normal ring handling (all LEDs on same driver)
    for (uint8_t i = 0; i < 14; i++)
    {
      uint8_t pwmValue;

      if (i < fullLeds)
      {
        pwmValue = 255;
      }
      else if (i == fullLeds)
      {
        pwmValue = partialPWM;
      }
      else
      {
        pwmValue = 0;
      }

      // Calculate LED address
      uint8_t ledAddress = ring.startLed + i;

      // Set PWM value on the appropriate driver
      if (ring.driver == 0)
      {
        Serial.println("INSIDE pwm ring == 0");
        Serial.print("ledAddress HEXA : ");
        Serial.println(ledAddress, HEX);
        Serial.print("ledAddress DEC : ");
        Serial.println(ledAddress, DEC);
        Serial.print("pwmValue HEX : ");
        Serial.println(pwmValue, HEX);
        
        selectPage(IS_1_ADDR, PWM_REG);
        setValue(IS_1_ADDR, ledAddress, pwmValue);
      }
      else
      {
        Serial.println("INSIDE pwm ring == 1");
        Serial.print("ledAddress HEXA : ");
        Serial.println(ledAddress, HEX);
        Serial.print("ledAddress DEC : ");
        Serial.println(ledAddress, DEC);
        Serial.print("pwmValue HEX : ");
        Serial.println(pwmValue, HEX);
        
        selectPage(IS_2_ADDR, PWM_REG);
        setValue(IS_2_ADDR, ledAddress, pwmValue);
      }
    }
  }
  Serial.println();
  Serial.println("      *****      ");
}

// Set mechanical switch state
void setSwitch(uint8_t switchNumber, bool switchValue)
{
  if (switchNumber >= NUM_SWITCHES) return;

  uint8_t pwmValue = switchValue ? 255 : 0;
  SwitchLEDs leds = switchLEDs[switchNumber];

  // Both LEDs are on IS2
  setValue(IS_2_ADDR, leds.led1, pwmValue);
  setValue(IS_2_ADDR, leds.led2, pwmValue);
}
