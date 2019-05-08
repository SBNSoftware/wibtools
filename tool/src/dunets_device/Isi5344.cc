
#include "dunets_device/Isi5344.hh"

Isi5344::Isi5344(I2CCore *i2c, uint8_t slaveaddr) {
  this->i2c = i2c;
  this->slaveaddr = slaveaddr;
}

// Read a specific register on the Si5344 chip. There is not check on the
// validity of the address but the code sets the correct page before reading.
size_t Isi5344::readRegister(uint16_t myaddr, uint8_t *res, size_t nwords) {
  // First make sure we are on the correct page
  uint8_t currentPg = getPage();
  uint8_t requirePg = (myaddr & 0xFF00) >> 8;
  if (currentPg != requirePg) setPage(requirePg);
  // Now read from register.
  uint8_t data[1] = {(uint8_t)(myaddr & 0xFF)};
  i2c->write(slaveaddr, data, 1, false);
  return i2c->read(slaveaddr, res, nwords);
}

// Write a specific register on the Si5344 chip. There is not check on the
// validity of the address but the code sets the correct page before reading.
// myaddr is an int
// data is a list of ints
void Isi5344::writeRegister(uint16_t myaddr, uint8_t *data, size_t n,
			    bool /*verbose*/) {
  // First make sure we are on the correct page
  uint8_t currentPg = getPage();
  uint8_t requirePg = (myaddr & 0xFF00) >> 8;
  if (currentPg != requirePg) setPage(requirePg);
  // Now write to register.
  //if (verbose) ...
  uint8_t *pdata = (uint8_t *)malloc(sizeof(uint8_t)*(n+1));
  pdata[0] = myaddr & 0xFF;
  memcpy((void*)&pdata[1], (void*)data, sizeof(uint8_t)*n);
  i2c->write(slaveaddr, pdata, n+1);
  free(pdata);
}

// Configure the chip to perform operations on the specified address page.
void Isi5344::setPage(uint8_t page, bool verbose) {
  uint8_t data[2] = {0x01, page};
  i2c->write(slaveaddr, data, 2, true);
  if (verbose) printf("Si5344 Set Reg Page: %u\n", page);
}

// Read the current address page
uint8_t Isi5344::getPage(bool verbose) {
  i2c->write(slaveaddr, (const uint8_t[]){0x01}, 1, false);
  uint8_t rPage;
  if (1 != i2c->read(slaveaddr, &rPage, 1)) return 0;
  if (verbose) printf("  Page read: %d\n", rPage);
  return rPage;
}

// Read registers containing chip information
void Isi5344::getDeviceVersion(uint8_t *res) {
  const size_t nwords = 2;
  setPage(0);
  i2c->write(slaveaddr, (const uint8_t[]){0x02}, 1, false);
  if (nwords != i2c->read(slaveaddr, res, 2)) return;
  printf("  CLOCK EPROM:\n \t  %#02x %#02x\n", res[0], res[1]);
}

#define DTS_CONFIG_PATH "DTS_CONFIG_PATH"                
int Isi5344::parse_clk_alloc(const char *filename,
    Si5344RegItem **bufptr, size_t *nptr) {
  printf("\tParsing file %s\n", filename);
  FILE *f = fopen(filename, "r");
  if (!f) {
    printf("SI5344 clock file error: %s not openable\n", filename);
    int errnoOriginal = errno;
    //Trying the ENV variable path
    char const * pathFromEnv = getenv(DTS_CONFIG_PATH);
    if(pathFromEnv != NULL){
      //The env variable is set, allocate a new buffer for it. 
      int buffer_size = snprintf(NULL,0,"%s/%s",pathFromEnv,filename);
      if(buffer_size > 0){
	char * filenameAlt = (char*) malloc(buffer_size+1);
	if(filenameAlt != NULL){
	  //fill new string with NULL terminators
	  memset(filenameAlt,0,buffer_size+1);
	  //create alternate path for files
	  snprintf(filenameAlt,buffer_size+1,"%s/%s",pathFromEnv,filename);
	  printf("Trying %s\n",filenameAlt);
	  f = fopen(filenameAlt,"r");
	}
	//delete the alternate filename string
	free(filenameAlt);
      }
    }
    
    //If we still have a bad file, return errno
    if(!f){
      return errnoOriginal;      
    }
  }
  char *line = NULL; // Let getline manage allocation
  size_t n = 0;
  size_t buflen = 1024;
  *bufptr = (__typeof__(*bufptr))malloc(sizeof(**bufptr)*buflen);
  *nptr = 0;
  bool header = false;
  int err = 0;
  while (-1 != getline(&line, &n, f)) {
    if (!n || !line || line[0]=='\n') break;
    if (line[0]=='#') continue;
    const char *headerline = "Address,Data";
    const size_t headern = strlen(headerline);
    if (0==strncmp(line, headerline, headern)) {
      if (!header) {
        header = true;
        continue;
      }
      printf("SI5344 clock file error: second header line\n");
      err = EINVAL;
    }
    if ((*nptr)+1 > buflen) {
      buflen*=2;
      *bufptr = (__typeof__(*bufptr))realloc(*bufptr, sizeof(**bufptr)*buflen);
    }
    if (2!=sscanf(line, "0x%04hx,0x%02hhx",
        &(*bufptr)[*nptr].addr, &(*bufptr)[*nptr].data ) ) {
      err = errno ? errno : EINVAL;
      goto error;
    }
    (*nptr)++;
  }
  if (line) free(line);
  printf("\t  %lu elements\n", (*nptr));
  return 0;
  error:
    if (line) free(line);
    if (*bufptr) {
      free(*bufptr); *bufptr = 0;
    }
    *nptr = 0;
    return err;
}

void Isi5344::writeConfiguration(Si5344RegItem *regSettingList, size_t n) {
  printf("\tWrite configuration:\n");
  size_t counter = 0;
  for (Si5344RegItem *reg = &regSettingList[0]; reg < &regSettingList[n];
      reg++) {
    //printf("\t  %lu Reg: 0x%x Data: [%u] 0x%x\n",
    //  counter, reg->addr, reg->data, reg->data);
    counter++;
    writeRegister(reg->addr, &reg->data, 1);
  }
}

