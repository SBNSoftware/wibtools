#ifndef __DUNETS_DEVICE_HPP__
#define __DUNETS_DEVICE_HPP__

//For tool device base class
#include "tool/CommandList.hh"

#include "tool/DeviceFactory.hh"

#include <string>
#include <vector>

#include "uhal/uhal.hpp"

#include <iostream>
#include <fstream>

#include "helpers/register_helper.hh"

#include "uhal_helpers/I2CuHal.hh"
#include "dunets_device/Isi5344.hh"

namespace WIBTool{
  
  class DUNETSDevice: public CommandList<DUNETSDevice>, public RegisterHelper{
  public:
    DUNETSDevice(std::vector<std::string> arg)
      : CommandList<DUNETSDevice>("DUNETS"),
	RegisterHelper(LOWER),
	hw(NULL),
	stream(NULL){
      //Setup this amc13
      Connect(arg);
      

      //Set the info string for this device
      SetInfo("");

      //setup commands
      LoadCommandList();
    }
 
    ~DUNETSDevice(){
      if(hw != NULL){
	delete hw;
      }
      if (uid_I2C) {
        delete uid_I2C;
        delete clock_I2C;
      }
    }

  private:
    uhal::HwInterface *hw;
    I2CCore *uid_I2C;
    I2CCore *clock_I2C;

    // address table path (a bit ugly, but has to go somewhere!)
    std::string addressTablePath;
    std::string connectionFile;

    std::ofstream* stream;
    std::string fileName;

    void Connect(std::vector<std::string> args);
    std::string Show();
    std::ofstream& getStream();
    bool isFileOpen() { return stream != NULL;}
    void setStream(const char* file);
    void closeStream();
    std::string getFileName() { return fileName; }
    void hostnameToIp(const char *hostname, char *ip);
    //    std::vector<std::string> myMatchNodes( uhal::HwInterface* hw, const std::string regex);

    //Implementation of read/write/myMatchRegex code
    std::vector<std::string> myMatchRegex(std::string regex);
    uint32_t RegReadAddress(uint32_t addr);
    uint32_t RegReadRegister(std::string const & addr);
    void RegWriteAction(std::string const & addr);
    void RegWriteAddress(uint32_t addr,uint32_t data);
    void RegWriteRegister(std::string const & addr, uint32_t data);

    uint32_t GetRegAddress(std::string const & reg);
    uint32_t GetRegMask(std::string const & reg);
    uint32_t GetRegSize(std::string const & reg);
    std::string GetRegMode(std::string const & reg);
    std::string GetRegPermissions(std::string const & reg);
    std::string GetRegDescription(std::string const & reg);
    std::string GetRegDebug(std::string const & reg);

    void BoardSetup();


    //Here is where you update the map between string and function
    void LoadCommandList();

    //Add new command functions here
    
    //   CommandReturn::status ListNodes(std::vector<std::string>,std::vector<uint64_t>);	   
        
    CommandReturn::status OpenFile(std::vector<std::string>,std::vector<uint64_t>);
    CommandReturn::status CloseFile(std::vector<std::string>,std::vector<uint64_t>);
    CommandReturn::status MrWuRegisterDump(std::vector<std::string>,std::vector<uint64_t>);
    CommandReturn::status CmdBoardSetup(std::vector<std::string> strArg,std::vector<uint64_t> intArg);

    //Add new command (sub command) auto-complete files here
    std::string autoComplete_Help(std::vector<std::string> const &,std::string const &,int);


  };
  RegisterDevice(DUNETSDevice,
		 "DTS",
		 "file/addr <Address table path> <prefix>",
		 "d",
		 "DTS",
		 "connection file/ip+table"
		 ); //Register DUNETSDevice with the DeviceFactory  
  static std::map<uint64_t, uint8_t> brd_rev = {
    {0xd880395e720b, 1},
    {0xd880395e501a, 1},
    {0xd880395e50b8, 1},
    {0xd880395e501b, 1},
    {0xd880395e7201, 1},
    {0xd880395e4fcc, 1},
    {0xd880395e5069, 1},
    {0xd880395e1c86, 2},
    {0xd880395e2630, 2},
    {0xd880395e262b, 2},
    {0xd880395e2b38, 2},
    {0xd880395e1a6a, 2},
    {0xd880395e36ae, 2},
    {0xd880395e2b2e, 2},
    {0xd880395e2b33, 2},
    {0xd880395e1c81, 2},

    {0xd880395e7206, 1},
  };
  static std::map<uint8_t, const char *> clk_cfg_files = {
    {1, "SI5344/PDTS0000.txt"},
    {2, "SI5344/PDTS0000.txt"},
  };
}

#endif
