#include "dunets_device/dunets_device.hh"
#include <WIBException/ExceptionBase.hh>
#include <boost/regex.hpp>
//For networking constants and structs                                                                          
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h> //for inet_ntoa        


using namespace WIBTool;

#define FILE_ADDR_ARG 0
#define ADDR_TABLE_PATH_ARG 1
#define PREFIX_ARG 2

void DUNETSDevice::Connect(std::vector<std::string> arg){

  //--- inhibit most noise from uHAL                                                                          
  uhal::setLogLevelTo(uhal::Error());


  if(0 == arg.size()){
    WIBException::DEVICE_CREATION_ERROR e;
    e.Append("bad args: DTS IP path/to/top.xml\n");
    throw e;
  }

  //We have a file/address
  connectionFile = arg[0];
  // Determine the address table path
  // 1st, non empty ADDR_TABLE_PATH_ARG
  // 2nd, enviornment variable DUNETS_ADDRESS_TABLE_PATH
  //      2.A  Leave alone if this ends in ".xml"
  //      2.B  Append top.xml if it doesn't end in ".xml"
  // 3rd, empty
  if((ADDR_TABLE_PATH_ARG < arg.size()) && (!arg[ADDR_TABLE_PATH_ARG].empty())){
    //An create argument that is non-empty
    addressTablePath = arg[ADDR_TABLE_PATH_ARG];
  } else if (NULL != getenv("DUNETS_ADDRESS_TABLE_PATH")){
    //Enviornment variable
    addressTablePath.assign(getenv("DUNETS_ADDRESS_TABLE_PATH"));
    //Append top.xml if the path doesn't end in an xml file
    if(addressTablePath.find(".xml") != (addressTablePath.size()-4)){
      addressTablePath+="/top.xml";
    }
  } else {
    addressTablePath.assign("");
  }

  // check for numeric IP address using Boost                                                                 
  // separate last octet so we can increment it                                                               
  static const boost::regex reIPAddress("(\\d{1,3}.\\d{1,3}.\\d{1,3}.)(\\d{1,3})(/[cC])?");
  static const boost::regex reXMLFile("\\S*\\.[xX][mM][lL]");
  static boost::cmatch reMatch;
  static char uri[256];


  if( boost::regex_match( connectionFile.c_str(), reMatch, reIPAddress)) {
    //=============================================================================
    //IP Address

    //Get the IP
    std::string ip_addr(reMatch[1].first, reMatch[1].second); // extract 1st 3 octets                               
    std::string ip_last(reMatch[2].first, reMatch[2].second); // extract last octet                                 
    uint8_t oct_last = atoi( ip_last.c_str());
    
    //Check for control hub option
    bool use_ch = reMatch[3].matched; // check for /c suffix                                                     
    if( use_ch){
      printf("use_ch true\n");
    } else {
      printf("use_ch false\n");
    }

    // specify protocol prefix                                                                                
    std::string proto = use_ch ? "chtcp-2.0://localhost:10203?target=" : "ipbusudp-2.0://";

    snprintf( uri, 255, "%s%s%d:50001", proto.c_str(), ip_addr.c_str(), oct_last);

    printf("Created URI from IP address:\n  %s\n",uri);

    std::string addrTableFull = "file://" + addressTablePath;

    printf("Address table name is %s\n", addrTableFull.c_str());

    try {
      hw = new uhal::HwInterface( uhal::ConnectionManager::getDevice("DUNETS", uri, addrTableFull));
    } catch( uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error creating uHAL hardware device\n");
    }

  } else if( boost::regex_match( connectionFile.c_str(), reMatch, reXMLFile) ) {
    //===========================================================================
    //Connection File  
    //===========================================================================
    printf("Left to the user as an exercise...Sorry\n");
//
//    printf("Using .xml connection file...\n");
//    if(3 >  arg.size()){ 
//      // the third argument is the prefix
//      // use default "T1" and "T2" xml IDs                                                     
//      amc13 = new AMC13( connectionFile);
//    } else { 
//      // specified xml ID prefix                                                                          
//      const std::string t1id = arg[PREFIX_ARG] + ".T1";
//      const std::string t2id = arg[PREFIX_ARG] + ".T2";
//      amc13 = new AMC13( connectionFile, t1id, t2id);
//  }
  } else { // hostname less '_t2' and '_t1'                                                                    
    //===========================================================================
    // Hostname
    //===========================================================================
    
    printf("does NOT match\n");
    // define necessary variables                                                                             
    char ip_t1[16];
    std::string name_t1;
    name_t1 = connectionFile;

    // convert hostname to ips                                                                                
    printf("converting host name...\n");
    hostnameToIp( name_t1.c_str(), ip_t1);
    printf("making proto...\n");

    // specify protocol prefix                                                                                
    std::string proto = "ipbusudp-2.0://"; // ATTENTION: may need to add check for chtcp protocol             

    printf("copying uris");
    // copy ip address to appropriate uris                                                                    
    snprintf( uri, 255, "%s%s:50001", proto.c_str(), ip_t1 );
    printf("Created URI from IP address: %s\n",uri);

    std::string addrTableFull = "file://" + addressTablePath;
    
    try {
      hw = new uhal::HwInterface( uhal::ConnectionManager::getDevice("DUNETS", uri, addrTableFull + "/dunets.xml"));
    } catch( uhal::exception::exception& e) {
      e.append("Module::Connect() creating hardware device");
      printf("Error creating uHAL hardware device\n");
    }

  }


  if( hw == NULL){
    WIBException::DEVICE_CREATION_ERROR e;
    e.Append("Unable to create HWInterface\n");
    throw e;    
  }

  // Create I2C Interfaces
  uid_I2C = new I2CCore(hw, 10, 5, "io.uid_i2c", 0);
  clock_I2C = new I2CCore(hw, 10, 5, "io.pll_i2c", 0);

}

void DUNETSDevice::BoardSetup() {

  uhal::ValWord<uint32_t> vreg = hw->getNode("io.csr.stat").read();
  hw->getNode("io.csr.ctrl.soft_rst").write(0x1);
  hw->dispatch();
  uint32_t reg = vreg;
  printf("reg: 0x%x\n", reg);
  sleep(1);
  hw->getNode("io.csr.ctrl.pll_rst").write(0x1);
  hw->dispatch();
  hw->getNode("io.csr.ctrl.pll_rst").write(0x0);
  hw->dispatch();
  uid_I2C->config();
  uid_I2C->write(0x21, (const uint8_t[]){0x01, 0x7f}, 2, true);
  uid_I2C->write(0x21, (const uint8_t[]){0x01}, 1, false);
  uint64_t id = 0; // 6 byte int
  do {
    { uint8_t res;
      if (1 != uid_I2C->read(0x21, &res, 1)) goto readerror;
      printf("I2c enable lines: %u\n", res);
      uid_I2C->write(0x53, (const uint8_t[]){0xfa}, 1, false);
    }
    { uint8_t res[6];
      if (6 != uid_I2C->read(0x53, res, 6)) goto readerror;
      for (uint8_t *i = &res[0]; i < &res[6]; i++) {
        id <<= 8; id |= *i;
      }
    }
    break;
    readerror:
      printf("Reply error in BoardSetup\n");
      return;
  } while(0);
  if (!brd_rev.count(id)) {
    printf("Unknown Board ID 0x%lx\n", id);
    return;
  }
  uint8_t rev = brd_rev[id];
  printf("Unique ID PROM / board rev: 0x%lx %u\n", id, rev);
  if (!clk_cfg_files.count(rev)) {
    printf("Unknown board rev %u\n", rev);
    return;
  }
  clock_I2C->config();
  Isi5344 *zeClock = new Isi5344(clock_I2C);
  uint8_t vers[2];
  zeClock->getDeviceVersion(vers);
  zeClock->setPage(0, true);
  zeClock->getPage(true);
  Si5344RegItem *regCfgList = NULL;
  size_t regCfgListN = 0;
  const char *clk_cfg_file = clk_cfg_files[rev];
  int err = zeClock->parse_clk_alloc(clk_cfg_file, &regCfgList, &regCfgListN);
  if (!regCfgList || err) {
    printf("Error parsing clock config file %s\n", clk_cfg_file);
    if (regCfgList) free(regCfgList);
    delete zeClock;
    return;
  }
  zeClock->writeConfiguration(regCfgList, regCfgListN);
  free(regCfgList);
  delete zeClock;
  for (uint32_t i=0; i<=1; i++) {
    hw->getNode("io.freq.ctrl.chan_sel").write(i);
    hw->dispatch();
    hw->getNode("io.freq.ctrl.en_crap_mode").write(0x0);
    hw->dispatch();
    sleep(2);
    uhal::ValWord<uint32_t> vfq = hw->getNode("io.freq.freq.count").read();
    uhal::ValWord<uint32_t> vfv = hw->getNode("io.freq.freq.valid").read();
    hw->dispatch();
    uint32_t fq = vfq;
    uint32_t fv = vfv;
    printf("fq = %d, fv = %d\n", fq, fv);
    printf("Freq: %u %u %f\n", i, fv, float(fq) * 119.20928 / 1000000. );
  }
  hw->getNode("io.csr.ctrl.sfp_tx_dis").write(0x0);
  hw->dispatch();

  hw->getNode("io.csr.ctrl.rst").write(0x1);
  hw->dispatch();
  hw->getNode("io.csr.ctrl.rst").write(0x0);
  hw->dispatch();

  uhal::ValWord<uint32_t> vstat = hw->getNode("io.csr.stat").read();
  hw->dispatch();
  uint32_t stat = vstat;
  printf("io.csr.stat : 0x%x\n", stat);
}

CommandReturn::status DUNETSDevice::CmdBoardSetup(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/) {
  BoardSetup();
  return CommandReturn::OK;
}


void DUNETSDevice::LoadCommandList(){
    // general commands (Launcher_commands)
  AddCommand("rd",&DUNETSDevice::Read,
	       "Read from dunets\n",
	       &DUNETSDevice::RegisterAutoComplete);
    AddCommand("wr",&DUNETSDevice::Write,
	       "Write to dunets\n",
	       &DUNETSDevice::RegisterAutoComplete);

    AddCommand("nodes", &DUNETSDevice::ListRegs, "List matching address table items\n");

    AddCommand("board_setup",&DUNETSDevice::CmdBoardSetup,
	       "dunets setup\n",
	       &DUNETSDevice::RegisterAutoComplete);

}



//If there is a file currently open, it closes it                                                             
void DUNETSDevice::setStream(const char* file) {
  if (stream != NULL) {
    stream->close();
    delete stream;
    stream = NULL;
  }
  stream = new std::ofstream;
  stream->open(file);
  fileName = file;
}

//Closes the stream                                                                                           
void DUNETSDevice::closeStream() {
  if (stream != NULL) {
    stream->close();
    delete stream;
    stream = NULL;
    fileName = "";
  }
}

// get ip from domain name                                                                                    
// ATTENTION should probably be void with throws instead of return 1 for failure, 0 for success               
void DUNETSDevice::hostnameToIp( const char *hostname, char *ip) { //change hostname to const str                   
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_in *h;
  int rv;

  printf("memset hints\n");
  memset(&hints, 0, sizeof hints);
  printf("memset hints complete\n");
  hints.ai_family = AF_INET; // use AF_INET assuming IPv4 or AF_UNSPEC if also accept IPv6                    
  hints.ai_socktype = SOCK_STREAM;

  if ( (rv = getaddrinfo( hostname , NULL , &hints , &servinfo) ) != 0) {
    // ATTENTION: should throw exception and append gai_strerror(rv), print and return 1 placeholder for now  
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return;
  }
  printf("start loop\n");
  // loop through all the results and connect to the first we can                                             
  for(p = servinfo; p != NULL; p = p->ai_next) {
    h = (struct sockaddr_in *) p->ai_addr;
    printf("strcpy\n");
    //ATTENTION: Should check that ip is large enough to copy given IP address                                
    strcpy(ip , inet_ntoa( h->sin_addr ) );
  }
  printf("free\n");
  freeaddrinfo(servinfo); // all done with this structure                                                     
}
