#include <wib_device/WIBDevice.hh>
#include <WIBStatus.hh>
#include <fstream>
#include <set>
#include <time.h> //time
#include <iomanip>

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

CommandReturn::status WIBTool::WIBDevice::Dump_WIB_FEMB_Status(std::vector<std::string> strArg,std::vector<uint64_t> intArg){ // Added by Varuna
 CommandReturn::status ret = CommandReturn::BAD_ARGS;
 
 if (intArg.size() != 1){ 
    std::cout << "Too many arguments.\n";
    return ret;
 }
 else if (intArg[0] > 4 || intArg[0] < 1){ 
   std::cout << "FEMB number " << intArg[0] << " is not valid. Valid FEMB numbers are 1, 2, 3, 4 \n";
   return ret;
 }
 
 else{
   std::map<std::string,double> map = wib->WIB_STATUS();
   
   if (map.empty()){
     std::cout << "Cannot get information for FEMB " << intArg[0] << "\n";
     return ret;
   }
 
   auto link_status = map.find("FEMB"+std::to_string(intArg[0])+"_LINK")->second;
   auto EQ_status = map.find("FEMB"+std::to_string(intArg[0])+"_EQ")->second;
   auto TS_link_status = map.find("FEMB"+std::to_string(intArg[0])+"_TS_LINK")->second;
   auto CHK_ERR_link_status = map.find("FEMB"+std::to_string(intArg[0])+"_CHK_ERR_LINK")->second;
   auto FRAME_ERR_link_status = map.find("FEMB"+std::to_string(intArg[0])+"_FRAME_ERR_LINK")->second;
   auto WIB_2991_VCC = map.find("WIB_2991_VCC")->second;
   auto WIB_2991_T = map.find("WIB_2991_T")->second;
   auto WIB_BIAS_V = map.find("WIB_BIAS_V")->second;
   auto WIB_BIAS_I = map.find("WIB_BIAS_I")->second;
   auto WIB_V18_V = map.find("WIB_V18_V")->second;
   auto WIB_V18_I = map.find("WIB_V18_I")->second;
   auto WIB_V36_V = map.find("WIB_V36_V")->second;
   auto WIB_V36_I = map.find("WIB_V36_I")->second;
   auto WIB_V28_V = map.find("WIB_V28_V")->second;
   auto WIB_V28_I = map.find("WIB_V28_I")->second;
   auto BIAS_2991_V = map.find("BIAS_2991_V")->second;
   auto BIAS_2991_T = map.find("BIAS_2991_T")->second;
   auto WIB_PC = map.find("WIB_PC")->second;
   auto FEMB_2991_VCC = map.find("FEMB"+std::to_string(intArg[0])+"_2991_VCC")->second;
   auto FEMB_2991_T = map.find("FEMB"+std::to_string(intArg[0])+"_2991_T")->second;
   auto FEMB_FMV39_V = map.find("FEMB"+std::to_string(intArg[0])+"_FMV39_V")->second;
   auto FEMB_FMV30_V = map.find("FEMB"+std::to_string(intArg[0])+"_FMV30_V")->second;
   auto FEMB_FMV18_V = map.find("FEMB"+std::to_string(intArg[0])+"_FMV18_V")->second;
   auto FEMB_BIAS_V = map.find("FEMB"+std::to_string(intArg[0])+"_BIAS_V")->second;
   auto FEMB_PC = map.find("FEMB"+std::to_string(intArg[0])+"_PC")->second;
   auto Bias_curr = map.find("FEMB"+std::to_string(intArg[0])+"_BIAS_I")->second;
   auto FMV39_curr = map.find("FEMB"+std::to_string(intArg[0])+"_FMV39_I")->second;
   auto FMV30_curr = map.find("FEMB"+std::to_string(intArg[0])+"_FMV30_I")->second;
   auto FMV18_curr = map.find("FEMB"+std::to_string(intArg[0])+"_FMV18_I")->second;
   auto AMV33_curr = map.find("FEMB"+std::to_string(intArg[0])+"_AMV33_I")->second;
   auto AMV28_curr = map.find("FEMB"+std::to_string(intArg[0])+"_AMV28_I")->second;
   auto AMV33_volt = map.find("FEMB"+std::to_string(intArg[0])+"_AMV33_V")->second;
   auto AMV28_volt = map.find("FEMB"+std::to_string(intArg[0])+"_AMV28_V")->second;
 
   std::string link_comment = link_status != 0xFF ? "Brocken":"Good";
   std::string eq_comment = EQ_status != 0xF ? "Brocken":"Good";
   std::string bias_curr_comment = (Bias_curr > 0.1 || Bias_curr < 0.001) ? "out of [0.001 - 0.1] range":"Good";
   std::string FMV39_curr_comment = (FMV39_curr > 0.2 || FMV39_curr < 0.010) ? "out of [0.010 - 0.2] range":"Good";
   std::string FMV30_curr_comment = (FMV30_curr > 0.5 || FMV30_curr < 0.050) ? "out of [0.050 - 0.5] range":"Good";
   std::string FMV18_curr_comment = (FMV18_curr > 1.0 || FMV18_curr < 0.200) ? "out of [0.2 - 1.0] range":"Good";
   std::string AMV33_curr_comment = (AMV33_curr > 1.0 || AMV33_curr < 0.100) ? "out of [0.1 - 1.0] range":"Good";
   std::string AMV28_curr_comment = (AMV28_curr > 1.5 || AMV28_curr < 0.100) ? "out of [0.1 - 1.5] range":"Good";
   
   std::cout << std::left << std::setw(20) << "Variable" << std::left << std::setw(20) << "Value" << std::left << std::setw(80) << "Comments" << "\n";
   std::cout << "****************************************************************************\n";
   std::cout << std::left << std::setw(20) << "Link status" << std::left << std::setw(20) << link_status << std::left << std::setw(80) 
             << link_comment << "\n";
   std::cout << std::left << std::setw(20) << "EQ status" << std::left << std::setw(20) << EQ_status << std::left << std::setw(80) 
             << eq_comment << "\n";
   std::cout << std::left << std::setw(20) << "TS link_status" << std::left << std::setw(20) << TS_link_status 
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "CHK ERR link status" << std::left << std::setw(20) << CHK_ERR_link_status 
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "FRAME ERR link status " << std::left << std::setw(20) << FRAME_ERR_link_status 
             << std::left << std::setw(80) << "---" << "\n"; 
   std::cout << std::left << std::setw(20) << "Bias Current" << std::left << std::setw(20) << std::to_string(Bias_curr) + " A" 
             << std::left << std::setw(80) << bias_curr_comment << "\n";
   std::cout << std::left << std::setw(20) << "Bias Voltage" << std::left << std::setw(20) << std::to_string(FEMB_BIAS_V) + " V" 
             << std::left << std::setw(80) << "---" << "\n";	     
   std::cout << std::left << std::setw(20) << "FM_V39 Current" << std::left << std::setw(20) << std::to_string(FMV39_curr) + " A"  
             << std::left << std::setw(80) << FMV39_curr_comment << "\n";
   std::cout << std::left << std::setw(20) << "FM_V39 Voltage" << std::left << std::setw(20) << std::to_string(FEMB_FMV39_V) + " V"  
             << std::left << std::setw(80) << "---" << "\n";	     
   std::cout << std::left << std::setw(20) << "FM_V30 Current" << std::left << std::setw(20) << std::to_string(FMV30_curr)+ " A"  
             << std::left << std::setw(80) << FMV30_curr_comment << "\n";
   std::cout << std::left << std::setw(20) << "FM_V30 Voltage" << std::left << std::setw(20) << std::to_string(FEMB_FMV30_V) + " V"  
             << std::left << std::setw(80) << "---" << "\n";	     
   std::cout << std::left << std::setw(20) << "FM_V18 Current" << std::left << std::setw(20) << std::to_string(FMV18_curr) + " A"
             << std::left << std::setw(80) << FMV18_curr_comment << "\n";
   std::cout << std::left << std::setw(20) << "FM_V18 Voltage" << std::left << std::setw(20) << std::to_string(FEMB_FMV18_V) + " V" 
             << std::left << std::setw(80) << "---" << "\n";	     
   std::cout << std::left << std::setw(20) << "AM_V33 Current" << std::left << std::setw(20) << std::to_string(AMV33_curr) + " A"
             << std::left << std::setw(80) << AMV33_curr_comment << "\n";
   std::cout << std::left << std::setw(20) << "AM_V33 Voltage" << std::left << std::setw(20) << std::to_string(AMV33_volt) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "AM_V28 Current" << std::left << std::setw(20) << std::to_string(AMV28_curr) + " A"
             << std::left << std::setw(80) << AMV28_curr_comment << "\n";
   std::cout << std::left << std::setw(20) << "AM_V28 Voltage" << std::left << std::setw(20) << std::to_string(AMV28_volt) + " V"  
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "FEMB 2991 VCC Voltage " << std::left << std::setw(20) << std::to_string(FEMB_2991_VCC) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "FEMB 2991 Temperature " << std::left << std::setw(20) << std::to_string(FEMB_2991_T) + " C" 
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "FEMB PC" << std::left << std::setw(20) << std::to_string(FEMB_PC) + " W"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << "\n";
   std::cout << "*******************************  WIB Parameters ******************************\n";
   std::cout << std::left << std::setw(20) << "WIB 2991 VCC Voltage " << std::left << std::setw(20) << std::to_string(WIB_2991_VCC) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB 2991 Temperature " << std::left << std::setw(20) << std::to_string(WIB_2991_T) + " C"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB BIAS Voltage" << std::left << std::setw(20) << std::to_string(WIB_BIAS_V) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB BIAS Current" << std::left << std::setw(20) << std::to_string(WIB_BIAS_I) + " A" 
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB V18 Voltage" << std::left << std::setw(20) << std::to_string(WIB_V18_V) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB V18 Current" << std::left << std::setw(20) << std::to_string(WIB_V18_I) + " A"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB V36 Voltage" << std::left << std::setw(20) << std::to_string(WIB_V36_V) + " V" 
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB V36 Current" << std::left << std::setw(20) << std::to_string(WIB_V36_I) + " A"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB V28 Voltage" << std::left << std::setw(20) << std::to_string(WIB_V28_V) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB V28 Current" << std::left << std::setw(20) << std::to_string(WIB_V28_I) + " A"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "BIAS 2991 Voltage" << std::left << std::setw(20) << std::to_string(BIAS_2991_V) + " V"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "BIAS 2991 Temperature " << std::left << std::setw(20) << std::to_string(BIAS_2991_T) + " C"
             << std::left << std::setw(80) << "---" << "\n";
   std::cout << std::left << std::setw(20) << "WIB PC" << std::left << std::setw(20) << std::to_string(WIB_PC) + " W" 
             << std::left << std::setw(80) << "---" << "\n";
   ret = CommandReturn::OK;
 }  
 
 return ret;
}
