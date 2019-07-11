#ifndef __MBB_HH__
#define __MBB_HH__
#define CRATE_COUNT 4
#include <stdint.h>
#include <string>
#include <AddressTable.hh>

// Keeps artDAQ from complaining about __int128
// not being in C++ standard
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

class MBB
{
 public:
  MBB(std::string const & address, std::string const & MBBAddressTable = "MBB.adt", bool fullStart=true);

  //initialize hardware
  void InitializeMBB();
  void ResetMBB(bool reset_udp=false);

  //PTC 
  void WIBPower(uint8_t icrate,bool turnOn); //to be used to extract bit mask for PTC_DATA
  void WritePTC(uint8_t icrate, uint16_t address, uint32_t value);
  void WritePTC(uint8_t icrate, std::string const & address, uint32_t value);  
  void ConfigPTC(uint8_t icrate);
  void ConfigAllPTCs();

  // Basic I/O methods
  uint32_t Read(uint16_t address);
  uint32_t Read(std::string const & address);
  void Write(uint16_t address,uint32_t value);
  void Write(std::string const & address,uint32_t value);
  void FullStart();
  AddressTable *map;

  Item const * GetItem(std::string const & str)
  { return map->GetItem(str);}

  std::vector<std::string> GetNames(std::string const & regex)
  { return map->GetNames(regex); }

 private:
  MBB(); //disallow the default constructor
  // Prevent copying of MBB objects
  MBB( const MBB& other) ; // prevents construction-copy
  MBB& operator=( const MBB&) ; // prevents copying
  bool started;
};
#endif
