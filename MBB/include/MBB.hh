#ifndef __MBB_HH__
#define __MBB_HH__
#define CRATE_COUNT 4
#define WIB_COUNT 6
#include <stdint.h>
#include <string>
#include <AddressTable.hh>



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
  //void WritePTC(uint8_t icrate, std::string const & address, uint32_t value);  
  void ConfigPTC(uint8_t icrate);
  void ConfigAllPTCs();

  // Basic I/O methods
  uint32_t Read(uint16_t address);
  uint32_t Read(std::string const & address);
  void Write(uint16_t address,uint32_t value);
  void Write(std::string const & address,uint32_t value);
  void FullStart();
  AddressTable *mbb;

  std::string GetAddress();

  Item const * GetItem(std::string const & str)
  { return mbb->GetItem(str);}

  std::vector<std::string> GetNames(std::string const & regex)
  { return mbb->GetNames(regex); }
  
  static const int Version; //SVN version
  int GetSVNVersion(){return Version;}

 private:
  MBB(); //disallow the default constructor
  // Prevent copying of MBB objects
  MBB( const MBB& other) ; // prevents construction-copy
  MBB& operator=( const MBB&) ; // prevents copying
  bool started;
};
#endif
