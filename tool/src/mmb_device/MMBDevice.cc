#include <mmb_device/MMBDevice.h>

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
    MMBTable = "MMB.adt";
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

