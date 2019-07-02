#include <mbb_device/MBBDevice.h>
#include <iostream>

using namespace std;

WIBTool::MBBDevice::MBBDevice(std::vector<std::string> arg)
  : CommandList<MBBDevice>("MBB"),mbb(NULL)
{    
  LoadCommandList();
  if (arg.size() < 1)
  {
    //MBBException::DEVICE_CREATION_ERROR e;
    //e.Append("Bad argument count");
    //throw e;
    //    std::cout << "Error opening MBB" << std::endl;
  }
  //Get address
  Address=arg[0];    
  //Set MBB address table
  if(arg.size() > 1)
  {
    //argument
    MBBTable = arg[1];
  }
  else
  {
    //default
    MBBTable = "MBB.adt";
  }

  mbb = new MBB(Address,MBBTable,true);    
  //  SetInfo(mbb->GetAddress().c_str());
}

WIBTool::MBBDevice::~MBBDevice()
{
  if(mbb != NULL)
  {
    delete mbb;
  }
}

void WIBTool::MBBDevice::LoadCommandList()
{
  AddCommand("read",&MBBDevice::Read,
	     "Read from MBB\n"			\
	     "  Usage:\n"			\
	     "  read <address>\n",
	     &MBBDevice::autoComplete_MBBAddressTable);
  AddCommandAlias("rd","read");

  AddCommand("write",&MBBDevice::Write,
	     "Write to MBB\n"			\
	     "  Usage:\n"			\
	     "  write <address> <value>\n",
	     &MBBDevice::autoComplete_MBBAddressTable);
  AddCommandAlias("wr","write");

  AddCommand("names",&MBBDevice::Names,
	     "List matching address table names."\
	     "  Usage:\n"\
	     "  names <regex>\n",
	     &MBBDevice::autoComplete_MBBAddressTable);
  AddCommandAlias("nodes","names");

}

std::string WIBTool::MBBDevice::autoComplete_MBBAddressTable(std::vector<std::string> const & line,
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
      registerName = mbb->GetNames(partialNameRegex);
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

CommandReturn::status WIBTool::MBBDevice::Names(std::vector<std::string> strArg,
						std::vector<uint64_t> intArg)
{
  std::cout << intArg.size() << std::endl;
  std::cout << strArg.size() << std::endl;
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::MBBDevice::Read(std::vector<std::string> strArg,
					       std::vector<uint64_t> intArg)
{
  if(1 == intArg.size())
  {
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0]))
    {
      //numeric
      uint32_t val = mbb->Read(intArg[0]);
      //printf("0x%04  0x%08\n",intArg[0],val);
      printf("0x%04LX:  0x%08LX\n", (long long unsigned int)intArg[0],(long long unsigned int)val);
    }
    else
    {
      //string
      std::vector<std::string> names = mbb->GetNames(strArg[0]);
      for(size_t iRead = 0; iRead < names.size();iRead++)
      {
      	uint32_t val = mbb->Read(names[iRead]);
       	printf("%s 0x%08X\n",names[iRead].c_str(),val);      
      }
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

	   
CommandReturn::status WIBTool::MBBDevice::Write(std::vector<std::string> strArg,
						std::vector<uint64_t> intArg)
{
  if(intArg.size() == 2)
  {
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0]))
    {      
      //numeric
      mbb->Write(intArg[0],intArg[1]);
      //printf("0x%04  0x%08\n",intArg[0],intArg[1]);
      printf("0x%04LX:  0x%08LX\n", (long long unsigned int)intArg[0], (long long unsigned int)intArg[1]);
    }
    else
    {
      //string
      mbb->Write(strArg[0],intArg[1]);
      //printf("%s 0x%08\n",strArg[0].c_str(),intArg[1]);      
      printf("%s 0x%08LX\n", strArg[0].c_str(),(long long unsigned int)intArg[1]);      
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

