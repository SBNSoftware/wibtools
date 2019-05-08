#ifndef ISI5344_HH_
#define ISI5344_HH_

#include "uhal_helpers/I2CuHal.hh"
#include <stdint.h>

typedef struct {
  uint16_t addr;
  uint8_t data;
} Si5344RegItem;

class Isi5344 {
 public:
  Isi5344(I2CCore *i2c, uint8_t slaveaddr=0x68);
  size_t readRegister(uint16_t myaddr, uint8_t *res, size_t nwords);
  void writeRegister(uint16_t myaddr, uint8_t *data, size_t n,
    bool verbose=false);
  void setPage(uint8_t page, bool verbose=false);
  uint8_t getPage(bool verbose=false);
  void getDeviceVersion(uint8_t *res);
  int parse_clk_alloc(const char *filename,
    Si5344RegItem **bufptr, size_t *nptr);
  // parse_clk
  // void writeConfiguration(size_t n, uint8_t *regSettingList);
  void writeConfiguration(Si5344RegItem *regSettingList, size_t n);
 private:
  I2CCore *i2c;
  uint8_t slaveaddr;
};

#endif
