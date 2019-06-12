#ifndef __WIBDEVICE_HPP__
#define __WIBDEVICE_HPP__

//For tool device base class
#include "tool/CommandList.hh"

#include "tool/DeviceFactory.hh"
#include <WIB.hh>
#include <WIBEvent.h>

namespace WIBTool{

class WIBDevice: public CommandList<WIBDevice>{
public:
  WIBDevice(std::vector<std::string> arg);   
  ~WIBDevice();
private:
  WIB * wib;
  std::string Address;
  std::string WIBTable;
  std::string FEMBTable;

  void PrintNames(std::vector<std::string> const & names,bool isWIB = true);

  //WIBEvent_t
  bool Event_Extract (WIBEvent_t *event, size_t event_size,
    std::vector<data_8b10b_t> data, WIB::WIB_DAQ_t DAQMode, size_t *startp);
  void Event_Print (FILE *f, WIBEvent_t *d);

  //Here is where you update the map between string and function
  void LoadCommandList();
  //IO
  CommandReturn::status Read(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status FEMBRead(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status FEMBWrite(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
	   
  CommandReturn::status Read_DTS_CDS_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DTS_CDS_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Read_DTS_SI5344_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DTS_SI5344_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DTS_SI5344_Config(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DTS_SI5344_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Read_DTS_SI5344_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status EnableDAQLink(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status EnableDAQLink_Lite(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status Read_DAQ_SI5342_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DAQ_SI5342_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DAQ_SI5342_Config(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_DAQ_SI5342_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Read_DAQ_SI5342_Page(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status EnableADC(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   


  //Status display
  CommandReturn::status StatusDisplay(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status StatusDisplayHTML(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status StatusDisplayFILE(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   

  //debugging
  CommandReturn::status ReadDAQLinkSpyBufferEvents(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status ReadDAQLinkSpyBuffer(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status ReadCDLinkSpyBuffer(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status ConfigWIBFakeData(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status SetFEMBFakeData(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status SetFEMBStreamSource(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status PrintFEMBCaptureInfo(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status DumpAddressTable(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status DumpFEMBAddressTable(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status PrintPDTSERRHistory(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  

  //Flash
  CommandReturn::status WriteFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ReadFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  //local Flash
  CommandReturn::status WriteLocalFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ReadLocalFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg);


  //INIT
  CommandReturn::status InitializeDTS(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ResetSI5342(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ResetWIB(std::vector<std::string>,std::vector<uint64_t> intArg);
  CommandReturn::status ResetWIBAndCfgDTS(std::vector<std::string>,std::vector<uint64_t> intArg);
  CommandReturn::status CheckedResetWIBAndCfgDTS(std::vector<std::string>,std::vector<uint64_t> intArg);
  CommandReturn::status StartStreamToDAQ(std::vector<std::string>,std::vector<uint64_t> intArg);
  
  //DTS
  CommandReturn::status PDTSInRunningState(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ResetPDTS(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ResetSI5344(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ConfigureDTSCDS(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status SelectSI5344(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status SelectSI5342(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

CommandReturn::status DTSStartSync(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  //FEMB  
  CommandReturn::status PowerFEMB(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status FEMBSource(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status ConfigFEMB(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status ConfigFEMBFakeData(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status ConfigFEMBMode(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status SetupFEMBExtClock(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status SetupFEMBASICs(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status SetupFPGAPulser(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status SetupASICPulserBits(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   

  //Names
  CommandReturn::status Names(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status FEMBNames(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  //Addresses
  CommandReturn::status Addresses(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status FEMBAddresses(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  CommandReturn::status TestUDP(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  std::string autoComplete_WIBAddressTable(std::vector<std::string> const & line,std::string const & currentToken ,int state);
  std::string autoComplete_FEMBAddressTable(std::vector<std::string> const & line,std::string const & currentToken ,int state);
  std::string autoComplete_FEMBNumber_FEMBAddressTable(std::vector<std::string> const & line,std::string const & currentToken ,int state);
  std::string autoComplete_WIBTables(std::vector<std::string> const & line,std::string const & currentToken ,int state);

  CommandReturn::status Read_QSFP_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write_QSFP_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   


};
  RegisterDevice(WIBDevice,
		 "WIB",
		 "address <WIB Address table path> <FEMB Address table path>",
		 "w",
		 "WIB",
		 "address"
		 ) //Register WIBDevice with the DeviceFactory  

}

#endif
