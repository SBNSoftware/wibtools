
#include "uhal_helpers/I2CuHal.hh"

I2CCore::I2CCore(uhal::HwInterface *target, int wclk, int i2cclk,
    const char *name, int delay) {
  this->target = target;
  this->name = (char *)malloc(strlen(name)+1);
  strcpy(this->name, name);
  size_t fmtlen = strlen(name)+128;
  char *fmtbuf = (char *)malloc(fmtlen);
  this->delay = delay;
  snprintf(fmtbuf, fmtlen, "%s.ps_lo", name);
  prescale_low = &target->getNode(fmtbuf);
  snprintf(fmtbuf, fmtlen, "%s.ps_hi", name);
  prescale_high = &target->getNode(fmtbuf);
  snprintf(fmtbuf, fmtlen, "%s.ctrl", name);
  ctrl = &target->getNode(fmtbuf);
  snprintf(fmtbuf, fmtlen, "%s.data", name);
  data = &target->getNode(fmtbuf);
  snprintf(fmtbuf, fmtlen, "%s.cmd_stat", name);
  cmd_stat = &target->getNode(fmtbuf);
  free(fmtbuf);
  wishboneclock = wclk;
  i2cclock = i2cclk;
  //config();
}

I2CCore::~I2CCore() {
  free(this->name);
}

void I2CCore::clearint() {
  ctrl->write(0x1);
  target->dispatch();
}

void I2CCore::config() {
  // INITIALIZATION OF THE I2S MASTER CORE
  // Disable core
  ctrl->write(0x0<<7);
  target->dispatch();
  int prescale = 0x0100; // FOR NOW HARDWIRED, TO BE MODIFIED
  prescale_low->write(prescale & 0xff);
  prescale_high->write((prescale & 0xff00) >> 8);
  // Enable core
  ctrl->write(0x1<<7);
  target->dispatch();
  printf("I2C config()ured\n");
}

bool I2CCore::checkack() {
  bool inprogress = true;
  bool ack = false;
  while (inprogress) {
    uhal::ValWord<uint32_t> stat = cmd_stat->read();
    target->dispatch();
    inprogress = (stat & I2CCore_CMDSTAT.inprogress);
    ack = ((stat & I2CCore_CMDSTAT.recvdack) == 0);
  }
  return ack;
}

bool I2CCore::delayorcheckack() {
  sleep(delay);
  return checkack();
}

// Write data to the device with the given address.
ssize_t I2CCore::write(uint8_t addr, const uint8_t *data, size_t n,
    bool stop) {
  // Start transfer with 7 bit address and write bit (0)
  ssize_t nwritten = -1;
  addr &= 0x7f;
  addr <<= 1;
  // Set transmit register (write operation, LSB=0)
  this->data->write(addr);
  // Set Command Register to 0x90 (write, start)
  cmd_stat->write(I2CCore_CMD.start | I2CCore_CMD.write);
  target->dispatch();
  if (!delayorcheckack()) {
    cmd_stat->write(I2CCore_CMD.stop);
    target->dispatch();
    return nwritten;
  }
  for (size_t i=0; i<n; i++) {
    // Write slave memory address
    this->data->write(data[i]);
    // Set Command Register to 0x10 (write)
    cmd_stat->write(I2CCore_CMD.write);
    target->dispatch();
    if (!delayorcheckack()) {
      cmd_stat->write(I2CCore_CMD.stop);
      target->dispatch();
      return nwritten;
    }
    nwritten++;
  }
  if (stop) {
    cmd_stat->write(I2CCore_CMD.stop);
    target->dispatch();
  }
  return nwritten;
}

// Read n bytes of data from the device with the given address.
size_t I2CCore::read(uint8_t addr, uint8_t *buf, size_t n) {
  // Start transfer with 7 bit address and read bit (1)
  addr &= 0x7f;
  addr <<= 1;
  addr |= 0x1; // read bit
  data->write(addr);
  cmd_stat->write(I2CCore_CMD.start | I2CCore_CMD.write);
  target->dispatch();
  if (!delayorcheckack()) {
    cmd_stat->write(I2CCore_CMD.stop);
    target->dispatch();
    return 0;
  }
  for (size_t i=0; i<n; i++) {
    if (i != n-1) {
      cmd_stat->write(I2CCore_CMD.read);
    } else {
      cmd_stat->write(I2CCore_CMD.read | I2CCore_CMD.ack | I2CCore_CMD.stop
        );
      target->dispatch();
    }
    delayorcheckack();
    uhal::ValWord<uint32_t> vval = data->read();
    target->dispatch();
    buf[i] = vval;
  }
  return n;
}
