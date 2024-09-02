#ifndef AnalogBinLogger_h
#define AnalogBinLogger_h
const size_t BLOCK_SIZE = 64;
//------------------------------------------------------------------------------
// First block of file.
const size_t PIN_NUM_DIM =
    BLOCK_SIZE - 3 * sizeof(uint32_t) - 2 * sizeof(uint8_t);
struct metadata_t {
  uint32_t adcFrequency;           // ADC clock frequency
  uint32_t cpuFrequency;           // CPU clock frequency
  uint32_t sampleInterval;         // Sample interval in CPU cycles.
  uint8_t recordEightBits;         // Size of ADC values, nonzero for 8-bits.
  uint8_t pinCount;                // Number of analog pins in a sample.
  uint8_t pinNumber[PIN_NUM_DIM];  // List of pin numbers in a sample.
};
//------------------------------------------------------------------------------
// Data block for 8-bit ADC mode.
const size_t DATA_DIM8 = (BLOCK_SIZE - 2 * sizeof(uint16_t)) / sizeof(uint8_t);
struct block8_t {
  uint16_t count;    // count of data values
  uint16_t overrun;  // count of overruns since last block
  uint8_t data[DATA_DIM8];
};
//------------------------------------------------------------------------------
// Data block for 10-bit ADC mode.
const size_t DATA_DIM16 =
    (BLOCK_SIZE - 2 * sizeof(uint16_t)) / sizeof(uint16_t);
struct block16_t {
  unsigned short count;    // count of data values
  unsigned short overrun;  // count of overruns since last block
  unsigned short data[DATA_DIM16];
};
#endif  // AnalogBinLogger_h
