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

  AddCommand("configPTC",&MBBDevice::ConfigPTC,
             "Configure one PTC \n"              \
             "  Usage:\n"                        \
             "  configPTC <crate_number 1...4>\n",
             &MBBDevice::autoComplete_MBBAddressTable);
  //AddCommandAlias("","");

  AddCommand("configAllPTCs",&MBBDevice::ConfigAllPTCs,
             "Configure All PTCs\n"              \
             "  Usage:\n"                        \
             "  configAllPTCs <>\n",
             &MBBDevice::autoComplete_MBBAddressTable);
  //AddCommandAlias("",""); 

  AddCommand("writePTC",&MBBDevice::WritePTC,
             "Write to PTC\n"                    \
             "  Usage:\n"                        \
             "  writePTC <crate_number 1...4> <address> <value>\n",
             &MBBDevice::autoComplete_MBBAddressTable);
  //AddCommandAlias("","");
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

void WIBTool::MBBDevice::PrintNames(std::vector<std::string> const & names)
{
  printf("Found %zd names\n",names.size());
  printf("  Name                                                        address         mask              mode\n");
  for(std::vector<std::string>::const_iterator name = names.begin();
      name != names.end();
      name++)
  {
    Item const * item = mbb->GetItem(*name);      

    printf("  %-58s (address: 0x%04LX  mask: 0x%08LX)", name->c_str(),
	   (long long unsigned int)item->address,
	   (long long unsigned int)item->mask);
    std::string mode;
    if(item->mode & Item::READ)  {mode+="r";}
    if(item->mode & Item::WRITE) {mode+="w";}
    if(item->mode & Item::ACTION){mode+="a";}
    printf(" %-3s\n",mode.c_str());

    //print description (if there is only one entry in names)
    if((names.size() == 1) && (item->user.find("description") != item->user.end()))
    {
      std::string description = item->user.find("description")->second; 
      printf("    Description:   ");
      boost::char_separator<char> sep("\n");
      boost::tokenizer<boost::char_separator<char> > tokens(description, sep);
      boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
      if(it != tokens.end())
      {
	printf("%s\n",(*it).c_str());
	it++;
      }
      for ( ;it != tokens.end();++it)
      {
	printf("        %s\n",(*it).c_str());
      }       
    }
  }
}


CommandReturn::status WIBTool::MBBDevice::Names(std::vector<std::string> strArg,
						std::vector<uint64_t> intArg)
{
  (void) intArg; // to make compiler not complain about unused args
  if(strArg.size() > 0)
  {
    PrintNames(mbb->GetNames(strArg[0]));
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
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

CommandReturn::status WIBTool::MBBDevice::ConfigPTC(std::vector<std::string> strArg,std::vector<uint64_t> intArg)

{
  (void) strArg; // to make compiler not complain about unused args
  if(intArg.size() == 1)
    {
      //crate number is first and only argument
      mbb->ConfigPTC(intArg[0]);

      return CommandReturn::OK;
    }
  return CommandReturn::BAD_ARGS;
}


CommandReturn::status WIBTool::MBBDevice::ConfigAllPTCs(std::vector<std::string> strArg, std::vector<uint64_t> intArg)

{
  (void) strArg; // to make compiler not complain about unused args 
  if(intArg.size() == 0)
    {
      //NO argument                                   
      mbb->ConfigAllPTCs();

      return CommandReturn::OK;
    }
  return CommandReturn::BAD_ARGS;
}

CommandReturn::status WIBTool::MBBDevice::WritePTC(std::vector<std::string> strArg, std::vector<uint64_t> intArg)
{
  if(intArg.size() == 3)
    {
      //A check can be introduced to keep the crate number in between 1-4 and to throw error otherwise.
      //Check if the argument is a numerical address or string                             
      if(isdigit(strArg[1][0]))
	{
	  //numeric                                                                          
	  mbb->WritePTC(intArg[0],intArg[1],intArg[2]);
	  // printf("%02  0x%04  0x%08\n",intArg[0],intArg[1],intArg[2]);                                    
	  printf("CRATE%02LX  0x%04LX:  0x%08LX\n",(long long unsigned int)intArg[0], (long long unsigned int)intArg[1], (long long unsigned int)intArg[2]);
	}
      else
	{
	  //string                                                                           
	  mbb->WritePTC(intArg[0],strArg[1],intArg[2]);
	  //printf("%02  %s 0x%08\n",intArg[0],strArg[1].c_str(),intArg[2]);                                
	  printf("CRATE%02LX  %s 0x%08LX\n",(long long unsigned int)intArg[0], strArg[1].c_str(),(long long unsigned int)intArg[2]);
	}
      return CommandReturn::OK;
    }
  return CommandReturn::BAD_ARGS;
}
