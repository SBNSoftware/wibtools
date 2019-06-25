#ifndef __MMB_HH__
#define __MMB_HH__

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

class MMB
{
 public:
  MMB(std::string const & address, std::string const & MMBAddressTable = "MMB.adt", bool fullStart=true);

  //initialize hardware
  void InitializeMMB();
  void ResetMMB(bool reset_udp=false);

  // Basic I/O methods
  uint32_t Read(uint16_t address);
  uint32_t Read(std::string const & address);
  void Write(uint16_t address,uint32_t value);
  void Write(std::string const & address,uint32_t value);
  void FullStart();
  AddressTable *map;

 private:
  MMB(); //disallow the default constructor
  // Prevent copying of MMB objects
  MMB( const MMB& other) ; // prevents construction-copy
  MMB& operator=( const MMB&) ; // prevents copying
  bool started;
};
#endif
