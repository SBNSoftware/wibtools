#ifndef __MMBDEVICE_HPP__
#define __MMBDEVICE_HPP__

//For tool device base class
#include "tool/CommandList.hh"
#include "tool/DeviceFactory.hh"
#include "helpers/register_helper.hh"
#include <MMB.hh>

namespace WIBTool
{

class MMBDevice: public CommandList<MMBDevice>
{
public:
  MMBDevice(std::vector<std::string> arg);   
  ~MMBDevice();
  void LoadCommandList();
  void PrintNames(std::vector<std::string> const & names,bool isMMB = true);

private:
  MMB * mmb;
  std::string Address;
  std::string MMBTable;

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

  std::string autoComplete_MMBAddressTable(std::vector<std::string> const & line,std::string const & currentToken ,int state);
  std::string autoComplete_MMBTables(std::vector<std::string> const & line,std::string const & currentToken ,int state);


};
  RegisterDevice(MMBDevice,
		 "MMB",
		 "address <MMB Address table path>",
		 "m",
		 "MMB",
		 "address"
		 ) //Register MMBDevice with the DeviceFactory  

}

#endif
