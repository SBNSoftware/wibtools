#ifndef __MBBDEVICE_HPP__
#define __MBBDEVICE_HPP__

//For tool device base class
#include "tool/CommandList.hh"

#include "tool/DeviceFactory.hh"
#include <MBB.hh>
//#include <WIBEvent.h>

namespace WIBTool{

class MBBDevice: public CommandList<MBBDevice>{
public:
  MBBDevice(std::vector<std::string> arg);   
  ~MBBDevice();
private:
  MBB * mbb;
  std::string Address;
  std::string MBBTable;

  void PrintNames(std::vector<std::string> const & names);
 
 void LoadCommandList();
  
  //IO
  CommandReturn::status Read(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status Write(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   

  //Status display
  CommandReturn::status StatusDisplay(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status StatusDisplayHTML(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   
  CommandReturn::status StatusDisplayFILE(std::vector<std::string> strArg,std::vector<uint64_t> intArg);	   

  //Names
  CommandReturn::status Names(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  //Addresses
  CommandReturn::status Addresses(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status TestUDP(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

  std::string autoComplete_MBBAddressTable(std::vector<std::string> const & line, std::string const & currentToken , int state);

  //PTC
  CommandReturn::status ConfigPTC(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status ConfigAllPTCs(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
  CommandReturn::status WritePTC(std::vector<std::string> strArg,std::vector<uint64_t> intArg);
};
  RegisterDevice(MBBDevice,
		 "MBB",
		 "address <MBB Address table path>",
		 "m",
		 "MBB",
		 "address"
		 ) //Register MBBDevice with the DeviceFactory  

}

#endif
