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

#define WIB_RESPONSE_PACKET_BUFFER_SIZE 4048
#define WIB_WR_BASE_PORT 32000
#define WIB_RD_BASE_PORT 32001
#define WIB_RPLY_BASE_PORT 32002


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
//  BNL_UDP():buffer_size(0),buffer(NULL),total_retry_count(0) {Clear();};
  BNL_UDP():reg(NULL),buffer_size(0),buffer(NULL),total_retry_count(0) {};
  ~BNL_UDP();

  void Setup(std::string const & address, uint16_t port_offset = 0); 
  bool Ready(){return connected;};

  void SetWriteAck(bool val){writeAck=val;};
  bool GetWriteAck(){return writeAck;};

  uint32_t ReadWithRetry(uint16_t address,uint8_t retry_count=10);
  uint32_t Read(uint16_t address);
  void WriteWithRetry(uint16_t address, uint32_t value, uint8_t retry_count=10);
  void Write(uint16_t address,uint32_t value);
  void Write(uint16_t address,std::vector<uint32_t> const & values);
  void Write(uint16_t address,uint32_t const * values, size_t word_count);

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
};


#endif
