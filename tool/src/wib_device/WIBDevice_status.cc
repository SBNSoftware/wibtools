#include <wib_device/WIBDevice.hh>
#include <WIBStatus.hh>
#include <fstream>
#include <set>
#include <time.h> //time

CommandReturn::status WIBTool::WIBDevice::DumpAddressTable(std::vector<std::string> strArg,std::vector<uint64_t> /*intArg*/){

  bool dumpToStdOut = false;
  if (strArg.size() > 0) {
     dumpToStdOut = (strArg[0] == "stdout");
  }

  time_t currentTime = time(NULL);
  const size_t filenameSize = 40;
  char filename[filenameSize+1];
  if( 0 == strftime(filename,filenameSize,
		    "RegDump-%Y-%m-%d-%H:%M:%S-%Z.txt",
		    localtime(&currentTime))){
    snprintf(filename,filenameSize,"%s","RegDump.txt");
  }
  FILE * outFile;
  if (dumpToStdOut) {
    outFile = stdout;
  }
  else {
    outFile = fopen(filename,"w");
    if(NULL == outFile){
      return CommandReturn::BAD_ARGS;
    }
    printf("Dumping register contents to %s\n",filename);
  }

  std::vector<std::string> names = wib->GetNames("*");
  std::set<uint16_t> address;
  for(std::vector<std::string>::iterator it = names.begin();
      it != names.end();
      it++){
    Item const * item = wib->GetItem(*it);
    address.insert(item->address);
  }
  for(std::set<uint16_t>::iterator it = address.begin();
      it != address.end();
      it++){
    fprintf(outFile,"0x%04X:    0x%08X\n", 
	    *it,wib->Read(*it));
  }
  if (!dumpToStdOut) fclose(outFile);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::DumpFEMBAddressTable(std::vector<std::string> strArg,std::vector<uint64_t> intArg){

  if (intArg.size() < 1)
  {
    std::cout << "Error: FEMB number 1-4 required" << std::endl;
    return CommandReturn::BAD_ARGS;
  }

  uint64_t iFEMB = intArg[0];
  if (iFEMB < 1 || iFEMB > 4)
  {
    std::cout << "Error: FEMB number 1-4 required, not  " << iFEMB << std::endl;
    return CommandReturn::BAD_ARGS;
  }

  bool dumpToStdOut = false;
  if (strArg.size() > 1) {
     dumpToStdOut = (strArg[1] == "stdout");
  }

  time_t currentTime = time(NULL);
  const size_t filenameSize = 40;
  char filename[filenameSize+1];
  if( 0 == strftime(filename,filenameSize,
		    "FEMBRegDump-%Y-%m-%d-%H:%M:%S-%Z.txt",
		    localtime(&currentTime))){
    snprintf(filename,filenameSize,"%s","FEMBRegDump.txt");
  }
  FILE * outFile;
  if (dumpToStdOut) {
    outFile = stdout;
  }
  else {
    outFile = fopen(filename,"w");
    if(NULL == outFile){
      return CommandReturn::BAD_ARGS;
    }
    printf("Dumping register contents to %s\n",filename);
  }

  std::vector<std::string> names = wib->GetFEMBNames("*");
  std::set<uint16_t> address;
  for(std::vector<std::string>::iterator it = names.begin();
      it != names.end();
      it++){
    Item const * item = wib->GetFEMBItem(iFEMB,*it);
    address.insert(item->address);
  }
  for(std::set<uint16_t>::iterator it = address.begin();
      it != address.end();
      it++){
    fprintf(outFile,"0x%04X:    0x%08X\n", 
	    *it,wib->ReadFEMB(iFEMB,*it));
  }
  if (!dumpToStdOut) fclose(outFile);
  return CommandReturn::OK;
}

static bool IsNumberHelper(const std::string & str){
  bool levelIsNumber = true;
  for(size_t iDigit = 0; iDigit < str.size();iDigit++){
    //Check if char is 0-9,'-',or '.'.
    levelIsNumber = levelIsNumber && (isdigit(str[iDigit]) || str[iDigit] == '-' || str[iDigit] == '.');
  }
  return levelIsNumber;
}

CommandReturn::status WIBTool::WIBDevice::StatusDisplay(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  //Create status display object
  WIBStatus * stat = new WIBStatus(wib);
  std::ostream& stream = std::cout;
  if(intArg.size()==0){
    //default to level 1 display
    stat->Report(1,stream);
  } else if(intArg.size()==1){
    // arg 0 is a string, either "power" or "femb"
    stat->Report(1,stream,strArg[0]);
  }
  /*
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
  */
  return CommandReturn::OK;    
}

CommandReturn::status WIBTool::WIBDevice::StatusDisplayHTML(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  //Create status display object
  WIBStatus * stat = new WIBStatus(wib);
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

CommandReturn::status WIBTool::WIBDevice::StatusDisplayFILE(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  //Create status display object
  WIBStatus * stat = new WIBStatus(wib);
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
