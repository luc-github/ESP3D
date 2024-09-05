// An example of an external SPI driver.
//
#include "SPI.h"  // Only required if you use features in the SPI library.
#include "SdFat.h"

#if SPI_DRIVER_SELECT == 3  // Must be set in SdFat/SdFatConfig.h

// SD chip select pin.
#define SD_CS_PIN SS

// This is a simple driver based on the the standard SPI.h library.
// You can write a driver entirely independent of SPI.h.
// It can be optimized for your board or a different SPI port can be used.
// The driver must be derived from SdSpiBaseClass.
// See: SdFat/src/SpiDriver/SdSpiBaseClass.h
class MySpiClass : public SdSpiBaseClass {
 public:
  // Activate SPI hardware with correct speed and mode.
  void activate() { SPI.beginTransaction(m_spiSettings); }
  // Initialize the SPI bus.
  void begin(SdSpiConfig config) {
    (void)config;
    SPI.begin();
  }
  // Deactivate SPI hardware.
  void deactivate() { SPI.endTransaction(); }
  // Receive a byte.
  uint8_t receive() { return SPI.transfer(0XFF); }
  // Receive multiple bytes.
  // Replace this function if your board has multiple byte receive.
  uint8_t receive(uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      buf[i] = SPI.transfer(0XFF);
    }
    return 0;
  }
  // Send a byte.
  void send(uint8_t data) { SPI.transfer(data); }
  // Send multiple bytes.
  // Replace this function if your board has multiple byte send.
  void send(const uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      SPI.transfer(buf[i]);
    }
  }
  // Save SPISettings for new max SCK frequency
  void setSckSpeed(uint32_t maxSck) {
    m_spiSettings = SPISettings(maxSck, MSBFIRST, SPI_MODE0);
  }

 private:
  SPISettings m_spiSettings;
} mySpi;
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(50), &mySpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(50), &mySpi)
#endif  // ENABLE_DEDICATED_SPI
SdFat sd;

//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  sd.ls(&Serial, LS_SIZE);
  Serial.println("Done");
}
//------------------------------------------------------------------------------
void loop() {}
#else  // SPI_DRIVER_SELECT
#error SPI_DRIVER_SELECT must be three in SdFat/SdFatConfig.h
#endif  // SPI_DRIVER_SELECT

