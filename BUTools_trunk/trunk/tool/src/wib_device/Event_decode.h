

#ifdef FELIX
#ifdef FNNAME
#error Conflict
#endif
#define FNNAME(name) name##_FELIX
#define COLBLOCKS FELIX_COLblocks
#endif
#ifdef RCE
#ifdef FNNAME
#error Conflict
#endif
#define FNNAME(name) name##_RCE
#define COLBLOCKS RCE_COLblocks
#endif
#ifndef FNNAME
#error DAQ Format not given
#endif


static void FNNAME(WIBEvent_read) (WIBEvent_t *d, const uint8_t *buf) {
  d->COLDATA_count = COLBLOCKS;

  d->StartOfFrame = buf[0];
#ifdef FELIX
  buf += 4;
#endif
  d->Version = (buf[1])      & ((1<<5)-1);
  d->FiberNo = (buf[1] >> 5) & ((1<<5)-1);
  d->CrateNo = (buf[2])      & ((1<<5)-1);
  d->SlotNo  = (buf[2] >> 5) & ((1<<5)-1);
  d->Reserved_0 = buf[3];
  d->Mismatch = (bool)(buf[4] & (1<<0));
  d->OutOfSync = (bool)(buf[4] & (1<<1));
  d->Reserved_1 = unaligned_read_le16(buf+4) >> 2;
  d->WIB_Errors = unaligned_read_le16(buf+6);
  d->Timestamp  = unaligned_read_le64(buf+8) & ((1ul<<63)-1);
  d->WIB_counter = unaligned_read_le16(buf+14) & ((1ul<<31)-1);
  d->Z_mode = (buf[15] >> 7) & 1;
  buf += 16;
  for (int i = 0; i < COLBLOCKS; i++) {
    WIBEvent_COLDATA_read(&d->COLDATA[i], buf);
    buf += 28*4;
  }
#ifdef RCE
  d->CRC = unaligned_read_le32(buf+0);
  d->CRClength = 32;
#endif
#ifdef FELIX
  d->CRC = (unaligned_read_le32(buf+0) >> 8) & ((1<<20)-1);
  d->CRClength = 20;
#endif
}



#undef FNNAME
#undef COLBLOCKS
