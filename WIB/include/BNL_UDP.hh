#ifndef __BNL_UDP_HH__
#define __BNL_UDP_HH__

#include <string>
#include <string.h>
#include <strings.h>
#include <vector>

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>

// Struct copied from FW_PRGM (gige.h)
typedef struct 
{
  int sock_recv;
  int sock_write;
  int sock_read;
  char client_ip_addr[512];
  struct sockaddr_in si_recv;
  struct sockaddr_in si_write;
  socklen_t          si_lenw;
  struct sockaddr_in si_read;
  socklen_t          si_lenr;
} gige_reg_t;

class BNL_UDP 
{
public:

  enum 
  {
    WIB_RESPONSE_PACKET_BUFFER_SIZE = 4048,
    WIB_WR_BASE_PORT   = 0x7D00 ,
    WIB_RD_BASE_PORT   = 0x7D01 ,
    WIB_RPLY_BASE_PORT = 0x7D10,
    MBB_WR_BASE_PORT   = 0x7D00 ,
    MBB_RD_BASE_PORT   = 0x7D01 ,
    MBB_RPLY_BASE_PORT = 0x7D02 ,
    N_FEMBS_PER_WIB   = 4,
    FEMB1_RW_BASE     = 0x7900,
    FEMB2_RW_BASE     = 0x7A00,
    FEMB3_RW_BASE     = 0x7B00,
    FEMB4_RW_BASE     = 0x7C00,
    FEMB1_REPLY_BASE  = 0x7910,
    FEMB2_REPLY_BASE  = 0x7A10,
    FEMB3_REPLY_BASE  = 0x7B10,
    FEMB4_REPLY_BASE  = 0x7C10,
  };

  BNL_UDP(bool isMBB_ = false):isMBB(isMBB_),reg(NULL),buffer_size(0),buffer(NULL),total_retry_count(0) 
  { buffer_size = 0; buffer= NULL; };
  ~BNL_UDP();

  void Setup(std::string const & address, uint16_t port_offset = 0); 
  bool Ready(){return connected;};

  void SetWriteAck(bool val){writeAck=val;};
  bool GetWriteAck(){return writeAck;};

  uint32_t ReadWithRetry(uint16_t address,uint16_t retry_count=10);
  uint32_t Read(uint16_t address, uint32_t *value);
  uint32_t WriteWithRetry(uint16_t address, uint32_t value, uint16_t retry_count=10);
  uint32_t Write(uint16_t address,uint32_t value);
  uint32_t Write(uint16_t address,std::vector<uint32_t> const & values);
  uint32_t Write(uint16_t address,uint32_t const * values, size_t word_count);

  std::string GetAddress(){return remoteAddress;};

  uint64_t GetRetryCount(){return total_retry_count;};

  gige_reg_t* gige_reg_init(const char *IP_address, char *iface);

private:  
  // Prevent copying of BNL_UDP objects
  BNL_UDP( const BNL_UDP& other) ; // prevents construction-copy
  BNL_UDP& operator=( const BNL_UDP&) ; // prevents copying

  void FlushSocket(int sock);

  //functions
  void Clear();
  void Reset();
  void ResizeBuffer(size_t size  = WIB_RESPONSE_PACKET_BUFFER_SIZE);
  
  bool writeAck;
  bool isMBB;
 
  gige_reg_t* reg;
  
  //Network addresses
  std::string remoteAddress;  
  int16_t readPort;
  int16_t writePort;
  int16_t replyPort;

  //Network sockets and sockaddrs
  bool connected;

  //Packet buffer
  size_t buffer_size;
  uint8_t *buffer;
  uint64_t total_retry_count;

  // FEMBs never return a write response, too hard to implement (Jack)
  bool isFEMB;
};


#endif
