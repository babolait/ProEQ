// Chip Select pins
#define CS_0 2  // For MCPs U2-U9
#define CS_1 3  // For MCPs U10-U11

// MCP23S17 Opcodes and registers
#define MCP23S17_WRITE 0x40
#define MCP23S17_READ  0x41

// Register addresses (BANK=0 mode)
#define IODIRA    0x00
#define IODIRB    0x01
#define IPOLA     0x02
#define IPOLB     0x03
#define GPINTENA  0x04
#define GPINTENB  0x05
#define GPPUA     0x0C
#define GPPUB     0x0D
#define GPIOA     0x12
#define GPIOB     0x13
#define IOCON     0x0A

// Register configuration values
#define IOCON_PARAMETERS   0b01111000  // BANK=0, sequential mode, hardware address enabled
#define IODIR_PARAMETERS   0xFF        // All pins as inputs
#define IPOL_PARAMETERS    0x00        // Normal polarity
#define GPINTEN_PARAMETERS 0x00        // Disable interrupts
#define GPPU_PARAMETERS    0x00        // Pull-ups disabled



// MCP Device Structure
struct MCPDevice 
{
  uint8_t csPin;
  uint8_t hwAddr;  // Hardware address (A2,A1,A0)
};

// MCP Device List (matches your hardware documentation)
MCPDevice mcpList[10] = 
{
  {CS_0, 0},  // U2: CS_0 0-0-0
  {CS_0, 1},  // U3: CS_0 0-0-1
  {CS_0, 2},  // U4: CS_0 0-1-0
  {CS_0, 3},  // U5: CS_0 0-1-1
  {CS_0, 4},  // U6: CS_0 1-0-0
  {CS_0, 5},  // U7: CS_0 1-0-1
  {CS_0, 6},  // U8: CS_0 1-1-0
  {CS_0, 7},  // U9: CS_0 1-1-1
  {CS_1, 1},  // U10: CS_1 0-0-1
  {CS_1, 0}   // U11: CS_1 0-0-0
};

// Encoder Mapping (18 encoders)
const uint8_t encoderMCP[18] = 
{
  9,  // Encoder_1 = MCP_U11 (index 9)
  0,  // Encoder_2 = MCP_U2  (index 0)
  0,  // Encoder_3 = MCP_U2  (index 0)
  2,  // Encoder_4 = MCP_U4  (index 2)
  1,  // Encoder_5 = MCP_U3  (index 1)
  1,  // Encoder_6 = MCP_U3  (index 1)
  2,  // Encoder_7 = MCP_U4  (index 2)
  3,  // Encoder_8 = MCP_U5  (index 3)
  3,  // Encoder_9 = MCP_U5  (index 3)
  5,  // Encoder_10 = MCP_U7 (index 5)
  4,  // Encoder_11 = MCP_U6 (index 4)
  4,  // Encoder_12 = MCP_U6 (index 4)
  5,  // Encoder_13 = MCP_U7 (index 5)
  6,  // Encoder_14 = MCP_U8 (index 6)
  6,  // Encoder_15 = MCP_U8 (index 6)
  8,  // Encoder_16 = MCP_U10 (index 8)
  7,  // Encoder_17 = MCP_U9 (index 7)
  7   // Encoder_18 = MCP_U9 (index 7)
};

const uint8_t encoderReg[18] = 
{
  GPIOA,  // Encoder_1 = GPA
  GPIOB,  // Encoder_2 = GPB
  GPIOA,  // Encoder_3 = GPA
  GPIOA,  // Encoder_4 = GPA
  GPIOB,  // Encoder_5 = GPB
  GPIOA,  // Encoder_6 = GPA
  GPIOB,  // Encoder_7 = GPB
  GPIOB,  // Encoder_8 = GPB
  GPIOA,  // Encoder_9 = GPA
  GPIOA,  // Encoder_10 = GPA
  GPIOB,  // Encoder_11 = GPB
  GPIOA,  // Encoder_12 = GPA
  GPIOB,  // Encoder_13 = GPB
  GPIOB,  // Encoder_14 = GPB
  GPIOA,  // Encoder_15 = GPA
  GPIOA,  // Encoder_16 = GPA
  GPIOB,  // Encoder_17 = GPB
  GPIOA   // Encoder_18 = GPA
};

// Switch Mapping (15 switches)
const uint8_t switchMCP[15] = 
{
  9, 9, 9, 9, 9, 9, 9, 9,  // Switches 1-8 on U11 (index 9)
  8, 8, 8, 8, 8, 8, 8       // Switches 9-15 on U10 (index 8)
};

const uint8_t switchReg[15] = 
{
  GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB,  // All on Port B
  GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB
};

const uint8_t switchBit[15] = 
{
  7, 0, 6, 1, 5, 2, 4, 3,  // Switch_1 to Switch_8 bit positions
  1, 0, 2, 3, 4, 5, 6      // Switch_9 to Switch_15 bit positions
};

// Function Prototypes
void writeMCP(uint8_t csPin, uint8_t addr, uint8_t reg, uint8_t value);
uint8_t readMCP(uint8_t csPin, uint8_t addr, uint8_t reg);
void initMCP(uint8_t csPin, uint8_t addr);
void initAllMCPs();
uint8_t getEncoder(uint8_t encoderNumber);
bool getSwitch(uint8_t switchNumber);
void enableSwitchPullups();

// Low-level SPI write to MCP23S17
void writeMCP(uint8_t csPin, uint8_t addr, uint8_t reg, uint8_t value) 
{
  digitalWrite(csPin, LOW);
  SPI.transfer(MCP23S17_WRITE | (addr << 1));  // Write command (0b0100xxx0)
  SPI.transfer(reg);                 // Register address
  SPI.transfer(value);               // Data byte
  digitalWrite(csPin, HIGH);
}

// Low-level SPI read from MCP23S17
uint8_t readMCP(uint8_t csPin, uint8_t addr, uint8_t reg) 
{
  uint8_t value;
  digitalWrite(csPin, LOW);
  SPI.transfer(MCP23S17_READ | (addr << 1));  // Set register pointer
  SPI.transfer(reg);
  value = SPI.transfer(0x00);           // Read data
  digitalWrite(csPin, HIGH);
  return value;
}

// Initialize a single MCP23S17
void initMCP(uint8_t csPin, uint8_t addr) 
{
  writeMCP(csPin, addr, IOCON, IOCON_PARAMETERS);
  writeMCP(csPin, addr, IODIRA, IODIR_PARAMETERS);
  writeMCP(csPin, addr, IODIRB, IODIR_PARAMETERS);
  writeMCP(csPin, addr, IPOLA, IPOL_PARAMETERS);
  writeMCP(csPin, addr, IPOLB, IPOL_PARAMETERS);
  writeMCP(csPin, addr, GPINTENA, GPINTEN_PARAMETERS);
  writeMCP(csPin, addr, GPINTENB, GPINTEN_PARAMETERS);
  writeMCP(csPin, addr, GPPUA, GPPU_PARAMETERS);
  writeMCP(csPin, addr, GPPUB, GPPU_PARAMETERS);
}

// Initialize all MCP23S17 expanders
void initAllMCPs() 
{
  for (uint8_t i = 0; i < 10; i++) 
  {
    initMCP(mcpList[i].csPin, mcpList[i].hwAddr);
  }
}

// Read absolute encoder value (1-18)
uint8_t getEncoder(uint8_t encoderNumber) 
{
  if (encoderNumber < 1 || encoderNumber > 18) return 0;
  
  uint8_t index = encoderNumber - 1;
  MCPDevice mcp = mcpList[encoderMCP[index]];
  return readMCP(mcp.csPin, mcp.hwAddr, encoderReg[index]);
}

// Read mechanical switch state (1-15)
// Returns true when switch is pressed (active low with pullups)
bool getSwitch(uint8_t switchNumber) 
{
  if (switchNumber < 1 || switchNumber > 15) return false;
  
  uint8_t index = switchNumber - 1;
  MCPDevice mcp = mcpList[switchMCP[index]];
  uint8_t portValue = readMCP(mcp.csPin, mcp.hwAddr, switchReg[index]);
  
  // Check specific bit (0 = pressed, 1 = released)
  return !(portValue & (1 << switchBit[index]));
}

// Enable internal pullups for mechanical switches
void enableSwitchPullups() 
{
  // U11 (index 9) - Port B
  writeMCP(mcpList[9].csPin, mcpList[9].hwAddr, GPPUB, 0xFF);
  
  // U10 (index 8) - Port B
  writeMCP(mcpList[8].csPin, mcpList[8].hwAddr, GPPUB, 0xFF);
}
