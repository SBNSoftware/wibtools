#ifndef __MBB_HH__
#define __MBB_HH__
#define CRATE_COUNT 4
#define WIB_COUNT 6
#include <stdint.h>
#include <string>
#include <AddressTable.hh>

class MBB{
 public:
  MBB(std::string const & address, std::string const & MBBAddressTable = "MBB.adt", bool fullStart=true);

  //initialize hardware
  void InitializeMBB();
  void ResetMBB(bool reset_udp=false);

  //PTC 
  void WIBPower(uint8_t icrate,bool turnOn); //to be used to extract bit mask for PTC_DATA
  void EnableWIBs(uint8_t icrate, uint32_t value);
  void WritePTC(uint8_t icrate, uint16_t address, uint32_t value);
  void WritePTC(uint8_t icrate, std::string const & address, uint32_t value);  
  void ConfigPTC(uint8_t icrate, uint32_t wib_pwr0, uint32_t wib_pwr1, uint32_t wib_pwr2, uint32_t wib_pwr3, uint32_t wib_pwr4, uint32_t wib_pwr5);
  
  uint32_t mask;
  void ConfigMBB(uint32_t PULSE_SOURCE, uint32_t PULSE_PERIOD);
  void TimeStampReset();
  void StartFEMBDaq();
  void StopFEMBDaq();
  void CalibrationPulse();
  void NewCalibrationPulse(); // latest version of CalibrationPulse() function as instructed by Shanshan. (03/11/2024, Varuna Meddage)
  void UseMBBPeriodicalCalibSrc(uint32_t pls_prd); // Implemented as instructed by Shanshan (03/11/2024, Varuna Meddage)
  // Implemented as instructed by Shanshan (03/11/2024, Varuna Meddage)
  // Makse sure to connect an external signal generator
  // to MBB LIMO (CALI) before using this function. You
  // can set the pulse period from the external signal generator.
  void UseSignalGeneratorAsCalibSrc();
  void SystemReset();
  void RegisterReset();
  void UDPReset();
  void AlgReset();

  // Basic I/O methods
  uint32_t Read(uint16_t address);
  uint32_t Read(std::string const & address);
  uint32_t ReadPTC(int icrate, uint16_t address);
  uint32_t ReadPTC(int icrate, std::string const & address);
  void Write(uint16_t address,uint32_t value);
  void Write(std::string const & address,uint32_t value);
  void FullStart();

  //for storing wib power values.
  uint32_t wibpoweroncrate[16];

  AddressTable * mbb;

  std::string GetAddress();

  Item const * GetItem(std::string const & str)
  { return mbb->GetItem(str);}

  std::vector<std::string> GetNames(std::string const & regex)
  { return mbb->GetNames(regex); }
  
  int GetSVNVersion(){return Version;}

  void SetContinueOnMBBRegReadError(bool enable);

 private:
  MBB(); //disallow the default constructor
  // Prevent copying of MBB objects
  MBB( const MBB& other) ; // prevents construction-copy
  MBB& operator=( const MBB&) ; // prevents copying

  static const int Version; //SVN version
  bool started;
  bool ContinueOnMBBRegReadError;
};

#endif
