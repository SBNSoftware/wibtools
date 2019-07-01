#include <mmb_device/MMBDevice.h>
#include <iostream>

using namespace std;

WIBTool::MMBDevice::MMBDevice(std::vector<std::string> arg)
  : CommandList<MMBDevice>("MMB"),mmb(NULL)
{    
  //LoadCommandList();
  if (arg.size() < 1)
  {
    //MMBException::DEVICE_CREATION_ERROR e;
    //e.Append("Bad argument count");
    //throw e;
    //    std::cout << "Error opening MMB" << std::endl;
  }
  //Get address
  Address=arg[0];    
  //Set MMB address table
  if(arg.size() > 1)
  {
    //argument
    MMBTable = arg[1];
  }
  else
  {
    //default
    MMBTable = "MBB.adt";
  }

  mmb = new MMB(Address,MMBTable,true);    
  //  SetInfo(mmb->GetAddress().c_str());
}

WIBTool::MMBDevice::~MMBDevice()
{
  if(mmb != NULL)
  {
    delete mmb;
  }
}

void WIBTool::MMBDevice::LoadCommandList()
{
  AddCommand("read",&MMBDevice::Read,
	     "Read from MMB\n"			\
	     "  Usage:\n"			\
	     "  read <address>\n",
	     &MMBDevice::autoComplete_MMBAddressTable);
  AddCommandAlias("rd","read");

  AddCommand("write",&MMBDevice::Write,
	     "Write to MMB\n"			\
	     "  Usage:\n"			\
	     "  write <address> <value>\n",
	     &MMBDevice::autoComplete_MMBAddressTable);
  AddCommandAlias("wr","write");

  AddCommand("names",&MMBDevice::Names,
	     "List matching address table names."\
	     "  Usage:\n"\
	     "  names <regex>\n",
	     &MMBDevice::autoComplete_MMBAddressTable);
  AddCommandAlias("nodes","names");

}

std::string WIBTool::MMBDevice::autoComplete_MMBAddressTable(std::vector<std::string> const & line,
							     std::string const & currentToken ,
							     int state)
{
  if(line.size() > 0)
  {
    if((line.size() > 1) && (currentToken.size() == 0))
    {
      return std::string("");
    }
    
    static std::vector<std::string> registerName;
    static size_t iRegister;
    
    //Reload lists if we are just starting out at state == 0
    if(state == 0)
    {
      std::string partialNameRegex(currentToken);
      partialNameRegex+="*";
      registerName = mmb->GetNames(partialNameRegex);
      iRegister = 0;
    }
    else
    {
      iRegister++;
    }

    for(;iRegister < registerName.size();iRegister++)
    {
      //Do the string compare and make sure the token is found a position 0 (the start)
      if(registerName[iRegister].find(currentToken) == 0)
      {	
	return registerName[iRegister];
      }
    }
  }
  return std::string("");
}

CommandReturn::status WIBTool::MMBDevice::Names(std::vector<std::string> strArg,
						std::vector<uint64_t> intArg)
{
  std::cout << intArg.size() << std::endl;
  std::cout << strArg.size() << std::endl;
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::MMBDevice::Read(std::vector<std::string> strArg,
					       std::vector<uint64_t> intArg)
{
  if(1 == intArg.size())
  {
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0]))
    {
      //numeric
      uint32_t val = mmb->Read(intArg[0]);
      //printf("0x%04  0x%08\n",intArg[0],val);
      printf("0x%04LX:  0x%08LX\n", (long long unsigned int)intArg[0],(long long unsigned int)val);
    }
    else
    {
      //string
      std::vector<std::string> names = mmb->GetNames(strArg[0]);
      for(size_t iRead = 0; iRead < names.size();iRead++)
      {
      	uint32_t val = mmb->Read(names[iRead]);
       	printf("%s 0x%08X\n",names[iRead].c_str(),val);      
      }
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

	   
CommandReturn::status WIBTool::MMBDevice::Write(std::vector<std::string> strArg,
						std::vector<uint64_t> intArg)
{
  if(intArg.size() == 2)
  {
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0]))
    {      
      //numeric
      mmb->Write(intArg[0],intArg[1]);
      //printf("0x%04  0x%08\n",intArg[0],intArg[1]);
      printf("0x%04LX:  0x%08LX\n", (long long unsigned int)intArg[0], (long long unsigned int)intArg[1]);
    }
    else
    {
      //string
      mmb->Write(strArg[0],intArg[1]);
      //printf("%s 0x%08\n",strArg[0].c_str(),intArg[1]);      
      printf("%s 0x%08LX\n", strArg[0].c_str(),(long long unsigned int)intArg[1]);      
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

