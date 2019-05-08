#ifndef I2CUHAL_HH_
#define I2CUHAL_HH_

#include <uhal/uhal.hpp>

class I2CCore {
 public:
  I2CCore(uhal::HwInterface *target, int wclk, int i2cclk, const char *name,
    int delay=0);
  ~I2CCore();
  void clearint();
  void config();
  bool checkack();
  bool delayorcheckack();
  ssize_t write(uint8_t addr, const uint8_t *data, size_t n, bool stop=true);
  size_t read(uint8_t addr, uint8_t *buf, size_t n);
  // write-read ?
 private:
  uhal::HwInterface * target;
  char * name;
  int delay;
  const uhal::Node *prescale_low;
  const uhal::Node *prescale_high;
  const uhal::Node *ctrl;
  const uhal::Node *data;
  const uhal::Node *cmd_stat;
  int wishboneclock;
  int i2cclock;

};

// C++ fails to inline these STATIC CONSTANTS unless they are placed outside
//   of class
  static const struct {
    uint32_t start;
    uint32_t stop;
    uint32_t read;
    uint32_t write;
    uint32_t ack;
    uint32_t intack;
  } I2CCore_CMD = {
    .start  = 0x1 << 7,
    .stop   = 0x1 << 6,
    .read   = 0x1 << 5,
    .write  = 0x1 << 4,
    .ack    = 0x1 << 3,
    .intack = 0x1 << 0,
  };
  static const struct {
    uint32_t recvdack;
    uint32_t busy;
    uint32_t arblost;
    uint32_t inprogress;
    uint32_t interrupt;
  } I2CCore_CMDSTAT = {
    .recvdack   = 0x1 << 7,
    .busy       = 0x1 << 6,
    .arblost    = 0x1 << 5,
    .inprogress = 0x1 << 1,
    .interrupt  = 0x1 << 0,
  };

#endif
