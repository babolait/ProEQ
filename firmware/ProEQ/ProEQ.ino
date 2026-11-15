/*
   ProEQ project

   18 8 bit absolute encoders read via 10 x MCP23s17 SPI expander
   270 led dimmed via the 2 x IS31FL3743 I2C led driver

   Formating the encoder value, clamp it and send it
   if the relative value (not the physical one) has changed.
*/

#include <LiquidCrystal.h>
#include <Wire.h>
#include <SPI.h>
#include "ACE_Functions.h"
#include "SPI_functions.h"
#include "IS_functions.h"
#include "ACE128map12345678.h"




LiquidCrystal lcd(10, 9, 8, 7, 6, 5, 4);


// MIDI Configuration
const int MIDI_CHANNEL = 1;
const int MIDI_BAUD_RATE = 31250;  // Baudrate for the 16u2 chip baudrate, arduino as midi = 31250b/s
const int DEBUG_BAUD_RATE = 9600;  // Baudrate for debugging purpose
byte midiBuffer[3];

// Single-turn encoder configuration (0-127 range)
const int ENCODER_MIN = 0;
const int ENCODER_MAX = 127;
const int ENCODER_RANGE = ENCODER_MAX - ENCODER_MIN + 1;  // 128 steps


// Variables for single-turn tracking for each encoder
int relativeValues[NUM_ENCODERS];
int lastRelativeValues[NUM_ENCODERS];
int lastEncoderValues[NUM_ENCODERS];
bool firstRead[NUM_ENCODERS];

// Variables for multi-turn tracking for each encoder
bool multiTurnMode[NUM_ENCODERS];  // Toggle between single and multi-turn modes per encoder
int multiTurnValues[NUM_ENCODERS];  // Initialized at midpoint (0-16383)
const int MULTI_TURN_MIN = 0;
const int MULTI_TURN_MAX = 16383;
const int MULTI_TURN_RANGE = MULTI_TURN_MAX - MULTI_TURN_MIN + 1;  //16384 steps


void setup()
{
  // Initialize serial communication for debugging or as midi device
  Serial.begin(DEBUG_BAUD_RATE);

  // SPI setup for MCP23S17
  pinMode(CS_0, OUTPUT);
  pinMode(CS_1, OUTPUT);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  digitalWrite(CS_0, HIGH);  // De-select chip
  digitalWrite(CS_1, HIGH);  // De-select chip
  SPI.begin();  // Initialize SPI bus

  // Initialize all MCP23S17 expanders
  initAllMCPs();

  // I2C setup for IS (IS31FL3743A - I2C Led driver)
  Wire.begin();  // Join I2C bus as master
  Wire.setClock(400000);  // Set I2C clock to 400kHz
  initIS(IS_2_ADDR);  // Initialize led driver 2
  initScalling(IS_2_ADDR);  // Set scaling for led driver 2

  // Initialize all encoder arrays
  for (int i = 0; i < NUM_ENCODERS; i++)
  {
    // Initialize single-turn values
    relativeValues[i] = 64;  // Start at midpoint
    lastRelativeValues[i] = 64;
    lastEncoderValues[i] = 0;
    firstRead[i] = true;

    // Initialize multi-turn values
    multiTurnMode[i] = true;  // Default to multi-turn mode
    multiTurnValues[i] = 8192;  // Start at midpoint
  }
  lcd.begin(16, 2);
  lcd.clear();

}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("ProEQ");
  lcd.setCursor(0, 1);

  while (1)
  {
    // Process all 18 encoders
    for (int encoderIndex = 0; encoderIndex < NUM_ENCODERS; encoderIndex++)
    {
      // Read current encoder position (encoder numbers are 1-based)
      // encoderMap is the unint8_t list to convert the gray code from absolute encoder output (from MCP expander) to natural 0-127
      int currentEncoderValue = (pgm_read_byte(encoderMap_12345678 + getEncoder(encoderIndex + 1)));

      if (firstRead[encoderIndex])
      {
        // First read - just store the value, don't calculate delta
        lastEncoderValues[encoderIndex] = currentEncoderValue;
        firstRead[encoderIndex] = false;
      }
      else if (currentEncoderValue != lastEncoderValues[encoderIndex])
      {
        // Calculate delta with wrap-around handling
        int delta = getEncoderDelta(currentEncoderValue, lastEncoderValues[encoderIndex]);

        // Update single-turn relative value (0-127)
        relativeValues[encoderIndex] += delta;

        // Clamp to valid range
        relativeValues[encoderIndex] = clampInt(relativeValues[encoderIndex], ENCODER_MIN, ENCODER_MAX);

        // Update multi-turn value if in multi-turn mode
        if (multiTurnMode[encoderIndex])
        {
          // Scale delta for 14-bit range (0-16383)
          // Each encoder step moves 16384/128 = 128 steps in multi-turn mode
          int scaledDelta = delta * (MULTI_TURN_RANGE / (int)ENCODER_RANGE);
          multiTurnValues[encoderIndex] += scaledDelta;

          // Clamp multi-turn value to valid range
          multiTurnValues[encoderIndex] = clampInt(multiTurnValues[encoderIndex], MULTI_TURN_MIN, MULTI_TURN_MAX);
        }

        // Store current value for next comparison
        lastEncoderValues[encoderIndex] = currentEncoderValue;
      }
      // If relativeValue changed then send midi CC and refresh I2C led driver with new value
      if (relativeValues[15] != lastRelativeValues[15])
      {
        // Send MIDI CC message
        // Using CC numbers 1-18 for each encoder (or adjust as needed)
        //sendMidiCC(16 + 1, relativeValues[15]);

        // Update lastRelativeValue to current value
        lastRelativeValues[15] = relativeValues[15];
        lcd.setCursor(0, 1);
        lcd.print("   ");
        lcd.setCursor(0, 1);
        lcd.print(relativeValues[encoderIndex]);
        lcd.setCursor(6, 1);
        lcd.print(encoderIndex);
        setEncoderRing(encoderIndex, relativeValues[encoderIndex]);
        // gimicke(127);

      }
      if (relativeValues[16] != lastRelativeValues[16])
      {
        // Send MIDI CC message
        // Using CC numbers 1-18 for each encoder (or adjust as needed)
        //sendMidiCC(16 + 1, relativeValues[15]);

        // Update lastRelativeValue to current value
        lastRelativeValues[16] = relativeValues[16];
        lcd.setCursor(10, 1);
        lcd.print("   ");
        lcd.setCursor(10, 1);
        lcd.print(relativeValues[encoderIndex]);
        lcd.setCursor(15, 1);
        lcd.print(encoderIndex);
        setEncoderRing(encoderIndex, relativeValues[encoderIndex]);
        // gimicke(127);

      }
    }
  }
  SPI.end();
}

/**
       Sends a MIDI Control Change message
       @param controller CC number (0-127)
       @param value CC value (0-127)
*/
void sendMidiCC(int controller, int value)
{
  // Set up MIDI CC message header (Control Change on specified channel)
  midiBuffer[0] = 0xB0 | (MIDI_CHANNEL - 1);  // Status byte
  midiBuffer[1] = controller & 0x7F;          // Controller number
  midiBuffer[2] = value & 0x7F;                // Value (ensure 7-bit)
  Serial.write(midiBuffer, 3);
}

/**
       Calculates encoder delta with proper wrap-around detection
       Handles transitions across the 0-127 boundary
       @param current Current encoder reading
       @param previous Previous encoder reading
       @return Signed delta (-127 to 127)
*/
int getEncoderDelta(int current, int previous)
{
  int delta = current - previous;

  // Handle wrap-around cases
  if (delta > ENCODER_RANGE / 2)
  {
    // Wrapped from high to low (e.g., 127 -> 0)
    delta -= ENCODER_RANGE;
  } else if (delta < -ENCODER_RANGE / 2)
  {
    // Wrapped from low to high (e.g., 0 -> 127)
    delta += ENCODER_RANGE;  // Positive delta
  }
  return delta;
}

/**
      Enables or disables multi-turn mode for a specific encoder
      @param encoderIndex Index of encoder (0-17)
      @param enable true for multi-turn, false for single-turn
*/
void setMultiTurnMode(int encoderIndex, bool enable)
{
  if (encoderIndex >= 0 && encoderIndex < NUM_ENCODERS)
  {
    multiTurnMode[encoderIndex] = enable;

    if (enable)
    {
      // Optional: Reset multi-turn value to midpoint when enabling
      // multiTurnValues[encoderIndex] = 8192;
    }
  }
}

/**
      Clamps an integer value between minimum and maximum bounds
      FIXED: Now properly handles negative values
      @param value Value to clamp
      @param minVal Minimum allowed value
      @param maxVal Maximum allowed value
      @return Clamped value within [minVal, maxVal]
*/
int clampInt(int value, int minVal, int maxVal)
{
  if (value < minVal) return minVal;
  if (value > maxVal) return maxVal;
  return value;
}


/*
void gimicke(byte valeur)
{
  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0xFE);        // enable master pointer
  Wire.write(0xC5);        // code to enable
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0xFD);        // control master register pointer
  Wire.write(0x00);        // pwm
  Wire.endTransmission();    // stop transmitting


  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0x3b);        // control master register pointer
  Wire.write(valeur);        // pwm
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0x3C);        // control master register pointer
  Wire.write(valeur);        // pwm
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0x23);        // control master register pointer
  Wire.write(valeur);        // pwm
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0x34);        // control master register pointer
  Wire.write(valeur);        // pwm
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0x35);        // control master register pointer
  Wire.write(valeur);        // pwm
  Wire.endTransmission();    // stop transmitting

  Wire.beginTransmission(IS_2_ADDR); // transmit to device 0x20
  Wire.write(0x46);        // control master register pointer
  Wire.write(valeur);        // pwm
  Wire.endTransmission();    // stop transmitting
}*/
