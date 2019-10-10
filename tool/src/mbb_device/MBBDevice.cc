#include <mbb_device/MBBDevice.h>
#include <iostream>
#include <MBBException.hh>
#include <MBBStatus.hh>
#include <fstream>
#include <time.h> //time
#include <helpers/StatusDisplay/StatusDisplay.hh>
#include <MBB.hh>


WIBTool::MBBDevice::MBBDevice(std::vector<std::string> arg)
  : CommandList<MBBDevice>("MBB"),mbb(NULL)
{    
  LoadCommandList();
  if (arg.size() < 1)
  {
    MBBException::DEVICE_CREATION_ERROR e;
    e.Append("Bad argument count");
    throw e;
    std::cout << "Error opening MBB" << std::endl;
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

  mbb = new MBB(Address,MBBTable);    
  SetInfo(mbb->GetAddress().c_str());
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

   AddCommand("status",&MBBDevice::StatusDisplay,
             "Show status display\n"    \
             "  Usage:\n"                                       \
             "  status <level> <table>\n",
             &MBBDevice::autoComplete_MBBAddressTable);

   AddCommand("html-status",&MBBDevice::StatusDisplayHTML,
             "Write status display to status.html\n"    \
             "  Usage:\n"                                       \
	      "  html-status <level> <table>\n");

   AddCommand("file-status",&MBBDevice::StatusDisplayFILE,
             "Write status display to status-<date>.dump\n"     \
             "  Usage:\n"                                       \
	     "  file-status <level> <table>\n");
  
   AddCommand("configMBB",&MBBDevice::ConfigMBB,
	     "Configure the MBB\n"				\
	     "  Usage:\n"					\
	     "  configMBB <PLL_CLOCK_TYPE 0-1> <PULSE_SOURCE 0-1> <PULSE_PERIOD 0-10000000>\n");
  
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
     //check for CRATE
       if((0 >= intArg[0]) && (4 < intArg[0]))
          {
	   printf("Bad CRATE#: %02LX",(long long unsigned int)intArg[0]);
	   return CommandReturn::OK;
          }
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
	  //mbb->WritePTC(intArg[0],strArg[1],intArg[2]);
	  //printf("%02  %s 0x%08\n",intArg[0],strArg[1].c_str(),intArg[2]);                                
	  printf("No PTC map available\n");
	  printf("CRATE%02LX  %s 0x%08LX\n",(long long unsigned int)intArg[0], strArg[1].c_str(),(long long unsigned int)intArg[2]);
	}
      return CommandReturn::OK;
    }
  return CommandReturn::BAD_ARGS;
}

static bool IsNumberHelper(const std::string & str){
  bool levelIsNumber = true;
  for(size_t iDigit = 0; iDigit < str.size();iDigit++){
    //Check if char is 0-9,'-',or '.'.                                     
    levelIsNumber = levelIsNumber && (isdigit(str[iDigit]) || str[iDigit] == '-' || str[iDigit] == '.');
  }
  return levelIsNumber;
  }


CommandReturn::status WIBTool::MBBDevice::StatusDisplay(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  //Create status display object                                           
  MBBStatus  * stat = new MBBStatus(mbb);
  std::ostream& stream = std::cout;
  if(intArg.size()==0){
    //default to level 1 display                                          
    stat->Report(1,stream);
  } else if(intArg.size()==1){
    //arg 0 should be an integer                                           
    if (IsNumberHelper(strArg[0])){
      stat->Report(intArg[0],stream);
    } else {
      std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
    }
  } else if(intArg.size() > 1){
    //arg 0 should be an integer                                         
    if (IsNumberHelper(strArg[0])){
      for(size_t iTable = 1; iTable < strArg.size();iTable++){
        stat->Report(intArg[0],stream,strArg[iTable]);
      }
    } else {
      std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
    }
  }
  return CommandReturn::OK;
  }

CommandReturn::status WIBTool::MBBDevice::StatusDisplayHTML(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  //Create status display object                                        
  MBBStatus * stat = new MBBStatus(mbb);
  stat->SetHTML();
  std::ofstream stream("status.html");
  if(intArg.size()==0){
    //default to level 1 display                                         
    stat->Report(1,stream);
  } else if(intArg.size()==1){
    //arg 0 should be an integer                                        
    if (IsNumberHelper(strArg[0])){
      stat->Report(intArg[0],stream);
    } else {
      std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
    }
  } else if(intArg.size() > 1){
    //arg 0 should be an integer                                          
    if (IsNumberHelper(strArg[0])){
      stat->Report(intArg[0],stream,strArg[1]);
    } else {
      std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
    }
  }
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::MBBDevice::StatusDisplayFILE(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  //Create status display object                                       
  MBBStatus * stat = new MBBStatus(mbb);
  time_t currentTime = time(NULL);
  const size_t filenameSize = 40;
  char filename[filenameSize+1];
  if( 0 == strftime(filename,filenameSize,
                    "Status-%Y-%m-%d-%H:%M:%S-%Z.txt",
                    localtime(&currentTime))){
    snprintf(filename,filenameSize,"%s","Status.txt");
  }
  std::ofstream stream(filename);
  if(stream.fail()){
    return CommandReturn::BAD_ARGS;
  }

  printf("Dumping status display to %s\n",filename);

  if(intArg.size()==0){
    //default to level 1 display                                 
    stat->Report(1,stream);
  } else if(intArg.size()==1){
    //arg 0 should be an integer                                         
    if (IsNumberHelper(strArg[0])){
      stat->Report(intArg[0],stream);
    } else {
      std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
    }
  } else if(intArg.size() > 1){
    //arg 0 should be an integer                                  
    if (IsNumberHelper(strArg[0])){
      stat->Report(intArg[0],stream,strArg[1]);
    } else {
      std::cout << "Error: \"" << strArg[0] << "\" is not a valid print level\n";
    }
  }
  return CommandReturn::OK;
  }

CommandReturn::status WIBTool::MBBDevice::ConfigMBB(std::vector<std::string> strArg, std::vector<uint64_t> intArg)
{
  (void) strArg;
  const size_t nArgs = intArg.size();

  if(nArgs < 3){
    std::cout << "Error: Not enough args to configMBB" << std::endl;
    return CommandReturn::BAD_ARGS;
  }

  if(intArg[0] > 1){
    std::cout << "Error: Pll clock type Setting --  " << intArg[0] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[1] > 1){
    std::cout << "Error: Invalid Pulse Source Setting --  " << intArg[1] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[2] > 10000000){
    std::cout << "Error: Invalid Pulse Period Setting --  " << intArg[2] << std::endl;
    return CommandReturn::BAD_ARGS;
  }

  uint32_t PLL_CLOCK_TYPE = intArg[0];
  uint32_t PULSE_SOURCE = intArg[1];
  uint32_t PULSE_PERIOD = intArg[2];
 
  mbb->ConfigMBB(PLL_CLOCK_TYPE, PULSE_SOURCE, PULSE_PERIOD);
  return CommandReturn::OK;
}
