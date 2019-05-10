#include <wib_device/WIBDevice.hh>

#include <stdint.h>
#include <stdio.h>
#include "helpers/int_unaligned.h"

#include <WIB/WIBEvent.h>

#define FPRINTIND(f, i, fmt, ...) \
  fprintf(f, "%*s" fmt, i, "", __VA_ARGS__)

#define FPRINTIND_(f, i, fmt) FPRINTIND(f, i, fmt "%s", "")

struct Char32 {
  char c[33];
};

static Char32 bin(int bits, uint16_t num) {
  Char32 string;
  string.c[0] = '\0';
  if (bits <= 0 || bits > 16) return string;
  for (int i = 0; i < bits; i++) {
    string.c[2*(bits-1-i)+0] = (num & (1<<i)) ? '1' : '0';
    string.c[2*(bits-1-i)+1] = ' ';
  }
  string.c[2*bits-1] = '\0';
  return string;
}

void WIBTool::WIBDevice::Event_Print (FILE *f, WIBEvent_t *d) {
  int l = 2;
  int COLblocks = d->COLDATA_count;
  FPRINTIND(f,l, "StartOfFrame: 0x%02x\n",   d->StartOfFrame);
  FPRINTIND(f,l, "Version: %d\n",            d->Version);
  FPRINTIND(f,l, "FiberNo: %d\n",            d->FiberNo);
  FPRINTIND(f,l, "CrateNo: %d\n",            d->CrateNo);
  FPRINTIND(f,l, "SlotNo:  %d\n",             d->SlotNo);
  FPRINTIND(f,l, "(reserved): bits %s\n", bin(8,d->Reserved_0).c);
  FPRINTIND(f,l, "Mismatch:   %s\n", d->Mismatch ? "YES":"NO");
  FPRINTIND(f,l, "OutOfSync:  %s\n", d->OutOfSync ? "YES":"NO");
  FPRINTIND(f,l, "(reserved): bits %s\n", bin(14,d->Reserved_1).c);
  FPRINTIND(f,l, "WIB Errors: 0x%04x\n",     d->WIB_Errors);
  FPRINTIND(f,l, "Z: %s\n", d->Z_mode==0 ?
    "Full timestamp mode" : "WIB counter mode");
  if (d->Z_mode==0) {
    FPRINTIND(f,l, "Timestamp: 0x%016lx\n",     d->Timestamp);
  } else {
    FPRINTIND(f,l, "WIB counter: 0x%04x\n",    d->WIB_counter);
  }
  for (int i = 0; i < COLblocks; i++) {
    FPRINTIND(f,l, "COLDATA %d:\n", i);
    FPRINTIND(f,l, "  S1_ErrC:    0x%1x\n",     d->COLDATA[i].S1_ErrC);
    FPRINTIND(f,l, "  S2_ErrC:    0x%1x\n",     d->COLDATA[i].S2_ErrC);
    FPRINTIND(f,l, "  (reserved): 0x%02x\n",   d->COLDATA[i].Reserved_0);
    FPRINTIND(f,l, "  Checksum_A: 0x%04x\n", d->COLDATA[i].Checksum_A);
    FPRINTIND(f,l, "  Checksum_B: 0x%04x\n", d->COLDATA[i].Checksum_B);
    FPRINTIND(f,l, "  Time Stamp: 0x%04x\n", d->COLDATA[i].Time_Stamp);
    FPRINTIND(f,l, "  Errors:     0x%04x\n", d->COLDATA[i].Errors);
    FPRINTIND(f,l, "  (reserved): 0x%04x\n", d->COLDATA[i].Reserved_1);
    FPRINTIND_(f,l,  "  Streams: H");
    for (int k = 0; k < 8; k++) { fprintf(f, " CH%d", k+1); }
    fprintf(f, "\n");
    for (int j = 0; j < 8; j++) {
      FPRINTIND(f,l, "           %1x", d->COLDATA[i].Stream[j].Header);
      for (int k = 0; k < 8; k++) {
        fprintf(f, " %03x", d->COLDATA[i].Stream[j].Channel[k]);
      }
      fprintf(f, "\n");
    }
  }
  FPRINTIND(f,l, "CRC-%u: 0x%0*x\n",
    d->CRClength, (d->CRClength-1)/4+1, d->CRC);
}

static void WIBEvent_COLDATA_read (WIBEvent_COLDATA_t *d, const uint8_t *buf) {
  d->S1_ErrC = (buf[0]     ) & ((1<<4)-1);
  d->S2_ErrC = (buf[0] >> 4) & ((1<<4)-1);
  d->Reserved_0 = buf[1];
  d->Checksum_A = buf[2] | ( (uint16_t)buf[4] << 8 );
  d->Checksum_B = buf[3] | ( (uint16_t)buf[5] << 8 );
  d->Time_Stamp = unaligned_read_le16(buf+6);
  d->Errors     = unaligned_read_le16(buf+8);
  d->Reserved_1 = unaligned_read_le16(buf+10);
  buf += 12;
  for (int i = 0; i < 8; i++) {
    d->Stream[i].Header = ( buf[i/2] >> (4*(i%2)) ) & ((1<<4)-1);
  }
  buf += 4;
  for (int i = 0; i < 8; i++) { // stream iterator
    for (int j = 0; j < 4; j++) { // channel pair iterator
      int o = i%2 + i/2*24 + j*6;
      uint32_t pair =   buf[o+0];
      pair |= (uint32_t)buf[o+2] << 8;
      pair |= (uint32_t)buf[o+4] << 16;
      d->Stream[i].Channel[2*j+0] = (pair      ) & ((1<<12)-1);
      d->Stream[i].Channel[2*j+1] = (pair >> 12) & ((1<<12)-1);
    }
  }
}

#define RCE
#include "Event_decode.h"
#undef RCE
#define FELIX
#include "Event_decode.h"
#undef FELIX

bool WIBTool::WIBDevice::Event_Extract (WIBEvent_t *event, size_t event_size,
    std::vector<data_8b10b_t> data, WIB::WIB_DAQ_t DAQMode, size_t *startp) {

  int COLblocks = -1;
  uint8_t SOFbyte = -1;
  size_t runlength = -1;
  switch (DAQMode) {
   case (WIB::RCE):
    COLblocks = RCE_COLblocks;
    SOFbyte = RCE_SOFbyte;
    // 4 word header, COLblocks blocks of 28, 1 word trailer
    runlength = 4*(4 + 28*COLblocks + 1);
    break;
   case (WIB::FELIX):
    COLblocks = FELIX_COLblocks;
    SOFbyte = FELIX_SOFbyte;
    // 5 word header, COLblocks blocks of 28, 1 word trailer
    runlength = 4*(5 + 28*COLblocks + 1);
    break;
   case (WIB::UNKNOWN):
    fprintf(stderr, "Unknown WIB DAQ mode\n");
    return false;
  }

  if (event_size !=
      sizeof(WIBEvent_t) + COLblocks * sizeof(WIBEvent_COLDATA_t)
    ) return false;

  FILE *output = stdout;

  size_t bytei = (*startp);
  if (bytei >= data.size()) {
    return false;
  }
  // Find start of control bit == 1
  while (bytei < data.size() && !data[bytei].k) {
    bytei++;
  }
  if (bytei - (*startp)) {
    fprintf(output, "Skipped %lu bytes\n", bytei - (*startp));
  }
  // Find end of control bit == 1
  while (bytei < data.size() && data[bytei].k) {
    fprintf(output, "Control byte: 0x%02x\n", data[bytei].data);
    bytei++;
  }
  if (bytei == data.size()) {
    (*startp) = bytei;
    return false;
  }
  size_t eventbegin = bytei-1; // Want the last byte with control bit == 1
  uint8_t controlbyte = data[eventbegin].data;
  if (controlbyte != SOFbyte) {
    fprintf(output, "Incorrect control byte 0x%02x for start of event:\n",
      controlbyte);
    (*startp) = bytei;
    return false;
  }
  // Find next control bit == 1
  while (bytei < data.size() && !data[bytei].k) {
    bytei++;
  }
  // Now bytei points either to a byte with control bit ==1 or to end of buffer
  // FELIX uses whole word starting with control bit == 1
  for (int i = 0; i < 4 && bytei < data.size(); i++) bytei++;

  size_t len = bytei - eventbegin;
  if (len < runlength) {
    if (bytei == data.size()) {
      fprintf(output, "End of stream; %lu remaining bytes skipped\n", len);
      (*startp) = bytei;
      return false;
    }
    fprintf(output, "Insufficient run length %lu\n", len);
    (*startp) = bytei;
    return false;
  }

  // This copying is avoidable if interleaved std::vector<data_8b10b_t> were
  //   replaced by class of non-interleaved vector of bool and vector of byte
  uint8_t *buffer = new uint8_t[runlength];
  for (size_t i = 0; i < runlength; i++) {
    buffer[i] = data[eventbegin + i].data;
  }
  if (DAQMode == WIB::RCE)   WIBEvent_read_RCE  (event, buffer);
  if (DAQMode == WIB::FELIX) WIBEvent_read_FELIX(event, buffer);
  delete[] buffer;

  if (len > runlength) {
    fprintf(output, "Excess %lu bytes following event\n",
      len - runlength);
  }

  (*startp) = bytei;
  return true;
}

