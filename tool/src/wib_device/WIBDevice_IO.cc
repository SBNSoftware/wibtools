#include <wib_device/WIBDevice.hh>
#include <readline/readline.h> //for rl_insert_text
#define __STDC_FORMAT_MACROS
#include <inttypes.h> // for PRI macros
#include <BNL_UDP_Exception.hh>

CommandReturn::status WIBTool::WIBDevice::WriteLocalFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments
  if(2 == intArg.size()){
    wib->WriteLocalFlash(intArg[0],intArg[1]);
    return CommandReturn::OK;
  }  
  return CommandReturn::BAD_ARGS;
}
CommandReturn::status  WIBTool::WIBDevice::ReadLocalFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused arguments
  if(1 == intArg.size()){
    printf("0x%04X: 0x%08X\n",(uint32_t) intArg[0], (uint32_t) wib->ReadLocalFlash(intArg[0]));
    return CommandReturn::OK;
  }  
  return CommandReturn::BAD_ARGS;
}


CommandReturn::status WIBTool::WIBDevice::TestUDP
    (std::vector<std::string> strArg, std::vector<uint64_t> intArg) {
  (void) strArg; // to make compiler not complain about unused arguments
  if(intArg.size() == 0){
    return CommandReturn::BAD_ARGS;
  }
  Item const * item = wib->GetItem("REG_TEST_0");
  for(uint32_t i = 0; i < intArg[0];i++){
    uint32_t reply = 0;
    try{
      wib->Write(item->address,i);
      reply = wib->Read(item->address);
      if(reply != i){
	printf("Error: 0x%08X != 0x%08X\n",reply,i);
      }
    }catch(WIBException::BAD_REPLY & e){
      printf("Failed after %u(%u) read/writes\n",i,reply);
    }
  }
  return CommandReturn::OK;
}
CommandReturn::status WIBTool::WIBDevice::ReadDAQLinkSpyBufferEvents
    (std::vector<std::string> strArg, std::vector<uint64_t> intArg) {
  uint8_t trigger_mode = 0;
  uint8_t iDAQLink = 0;

  FILE *output = NULL;
  switch (intArg.size()){
  case 3:
    {
      const char *fileName = strArg[2].c_str();
      output = fopen(fileName, "w");
      if (!output) {
        fprintf(stderr, "Unable to open %s\n", fileName);
        return CommandReturn::BAD_ARGS;
      }
    }
  case 2:
    trigger_mode = intArg[1];
  case 1:
    iDAQLink = intArg[0];
    break;
  default:
    return CommandReturn::BAD_ARGS;    
  }  

  std::vector<data_8b10b_t> data =
    wib->ReadDAQLinkSpyBuffer(iDAQLink,trigger_mode);
  printf("Recorded %zu samples\n",data.size());

  WIB::WIB_DAQ_t DAQMode = wib->GetDAQMode();
  int COLblocks;
  if (DAQMode == WIB::RCE) COLblocks = RCE_COLblocks;
  if (DAQMode == WIB::FELIX) COLblocks = FELIX_COLblocks;
  size_t event_size =
    sizeof(WIBEvent_t) + COLblocks * sizeof(WIBEvent_COLDATA_t);
  WIBEvent_t *event = (WIBEvent_t *) calloc(1, event_size);
  size_t start = 0;
  while (start < data.size()) {
    bool extracted = Event_Extract(event, event_size, data, DAQMode, &start);
    if (extracted) {
      Event_Print(stdout, event);
      if (output) {
        fprintf(output, "Event:\n");
        Event_Print(output, event);
      }
    }
  }
  free(event);

  if (output) fclose(output);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ReadCDLinkSpyBuffer(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  FILE* outFile = NULL;
  switch (strArg.size()){
  case 2:
    if (strArg.size()){
      outFile = fopen(strArg[0].c_str(),"w");    
    }
  case 1:
    //Set stream ID
    if(intArg[0] < 16){
      wib->Write("FEMB_SPY.STREAM_ID",intArg[0]);
      wib->Write("FEMB_SPY.ARM",1);
      wib->Write("FEMB_SPY.SW_TRIG",1);
    }else{
      printf("Bad stream id %zu\n",intArg[0]);
      return CommandReturn::BAD_ARGS;      
    }
    break;
  default:
    return CommandReturn::BAD_ARGS;
    break;
  }

  std::vector<data_8b10b_t> data = wib->ReadOutCDLinkSpyBuffer();
  printf("Recorded %zu samples\n",data.size());
  for(size_t iWord = 0; iWord < data.size();iWord++){
    printf("  %04zu: %01X 0x%02X\n",
	   iWord,
	   data[iWord].k,
	   data[iWord].data);
    if(outFile){
      fprintf(outFile,"  %04zu: %01X 0x%04X\n",
	   iWord,
	   data[iWord].k,
	   data[iWord].data);
    }
  }    
  if(outFile){
    fclose(outFile);
  }

  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ReadDAQLinkSpyBuffer(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  uint8_t trigger_mode = 0;
  uint8_t iDAQLink = 0; 
  std::string fileName;
  switch (intArg.size()){
  case 3:
    fileName=strArg[2];
  case 2:
    trigger_mode = intArg[1];
  case 1:
    iDAQLink = intArg[0];
    break;
  default:
    return CommandReturn::BAD_ARGS;    
  }  
  
  FILE* outFile = NULL;
  if(fileName.size() > 0){
    outFile = fopen(fileName.c_str(),"w");
  }


  std::vector<data_8b10b_t> data = wib->ReadDAQLinkSpyBuffer(iDAQLink,trigger_mode);
  printf("Recorded %zu samples\n",data.size());
  if(1 == trigger_mode){
    //Find the trigger and align to that
    size_t offset = 0;
    for(; offset < data.size();offset++){
      if((1 == data[offset].k) && (0xBC == data[offset].data)){
	break;
      }
    }

    for(size_t iWord = offset; iWord < data.size();iWord+=4){
      printf("  %04zu: %01X 0x%08X\n",
	     (iWord-offset)>>2,
	     (data[iWord+0].k << 0    | data[iWord+1].k << 1    | data[iWord+2].k << 2     | data[iWord+3].k << 3 ),
	     (data[iWord+0].data << 0 | data[iWord+1].data << 8 | data[iWord+2].data << 16 | data[iWord+3].data << 24 ));      
      if(outFile){
	fprintf(outFile,"  %04zu: %01X 0x%08X\n",
		(iWord-offset)>>2,
		(data[iWord+0].k << 0    | data[iWord+1].k << 1    | data[iWord+2].k << 2     | data[iWord+3].k << 3 ),
		(data[iWord+0].data << 0 | data[iWord+1].data << 8 | data[iWord+2].data << 16 | data[iWord+3].data << 24 ));      
      }
    }    
  }else{
    for(size_t iWord = 0; iWord < data.size();iWord++){
      printf("  %03zu: %01u 0x%02X\n",iWord,data[iWord].k,data[iWord].data);
    }
  }

  if(outFile){
    fclose(outFile);
  }

  return CommandReturn::OK;    
}

static bool ok_to_continue( const char *message) {
  char buff[5];
  printf("%s\n", message);
  printf("Enter 'Yes' to continue-->");
  fgets( buff, 4, stdin);
  if( !strncmp( buff, "Yes", 3))
    return true;
  else
    return false;
}

CommandReturn::status WIBTool::WIBDevice::WriteFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(1 == intArg.size()){
    if( !ok_to_continue("EXPERT ONLY! This may result in a non-functioning WIB\n" \
			"Once the flash is erased the WIB will not power up \n"\
			"unless a valid firmware file is successfully loaded.\n"\
			"DO NOT power cycle the WIB until this successfully finishes!\n")){
      printf("Flash NOT programmed\n");
      return CommandReturn::OK;
    }
    wib->ProgramFlash(strArg[0],2);
    return CommandReturn::OK;
  }  
  else if(2 == strArg.size() && strArg[0] == "yesdoitnow"){
    wib->ProgramFlash(strArg[1],2);
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

CommandReturn::status WIBTool::WIBDevice::ReadFlash(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(1 == intArg.size()){
    wib->ReadFlash(strArg[0],2);
    return CommandReturn::OK;
  }  
  return CommandReturn::BAD_ARGS;
}


CommandReturn::status WIBTool::WIBDevice::Read(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(1 == intArg.size()){
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0])){
      //numeric
      uint32_t val = wib->Read(intArg[0]);
      printf("0x%04"PRIX64": 0x%08"PRIX32"\n",intArg[0],val);
    }else{
      //string
      std::vector<std::string> names = wib->GetNames(strArg[0]);
      for(size_t iRead = 0; iRead < names.size();iRead++){
	uint32_t val = wib->Read(names[iRead]);
	printf("%s: 0x%08X\n",names[iRead].c_str(),val);      
      }
    }
    return CommandReturn::OK;
  }else if (2 == intArg.size()){
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0])){
      if((intArg[0] >> 16) || //address is out of range
	 (intArg[1] >> 16) || //count is out of range
	 ((intArg[0] + intArg[1])>>16)){ //address + count it out of range
	printf("Address out of range\n");
      }else{
	//numeric
	for(uint16_t address = intArg[0]; address < (intArg[0] + intArg[1]);address++){
	  uint32_t val = wib->Read(intArg[0]);
	  printf("0x%04"PRIX16": 0x%08"PRIX32"\n",address,val);
	}
      }
    }else{
      //string name
      //Get address for that string and then read out the next 100 entries
      Item const * item = wib->GetItem(strArg[0]);
      if((item->address >> 16) || //address is out of range
	 (intArg[1] >> 16) || //count is out of range
	 ((uint64_t(item->address) + intArg[1])>>16)){ //address + count it out of range
	printf("Address out of range\n");
      }else{
	printf("%s @ 0x%"PRIX16" + %"PRIu64"\n",strArg[0].c_str(),item->address,intArg[1]);      
	for(uint16_t address = item->address;
	    address < item->address + uint16_t(intArg[1]);
	    address++){
	  uint32_t val = wib->Read(address);
	  printf("0x%04"PRIX16": 0x%08"PRIX32"\n",address,val);
	}
      }
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

	   
CommandReturn::status WIBTool::WIBDevice::Write(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(intArg.size() == 2){
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[0][0])){      
      //numeric
      wib->Write(intArg[0],intArg[1]);
      printf("0x%04"PRIX64": 0x%08"PRIX64"\n",intArg[0],intArg[1]);
    }else{
      //string
      wib->Write(strArg[0],intArg[1]);
      printf("%s: 0x%08"PRIX64"\n",strArg[0].c_str(),intArg[1]);      
    }
    return CommandReturn::OK;
  }else if(intArg.size() > 2){
    //Copy the write values into a vector for the block write command
    std::vector<uint32_t> values;
    for(size_t iVal = 1; iVal < intArg.size();iVal++){
      if((intArg[iVal] >> 32) != 0){
	printf("Warning: value %016"PRIX64" is bigger than 32bits\n",intArg[iVal]);
	printf("         Truncating to %08"PRIX32"\n",uint32_t(intArg[iVal]));
      }
      values.push_back(uint32_t(intArg[iVal]));
    }
    //Call write
    if(isdigit(strArg[0][0])){      
      //numeric
      wib->Write(intArg[0],values);
      printf("Writing %zu words to 0x%04"PRIX64"\n",values.size(),intArg[0]);
    }else{
      //string
      wib->Write(strArg[0],values);
      printf("Writing %zu words to %s\n",values.size(),strArg[0].c_str());
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}


void WIBTool::WIBDevice::PrintNames(std::vector<std::string> const & names,bool isWIB)
{
  printf("Found %zd names\n",names.size());
  printf("  Name                                                        address         mask              mode\n");
  for(std::vector<std::string>::const_iterator name = names.begin();
      name != names.end();
      name++){
    Item const * item;
    if(isWIB){
      item = wib->GetItem(*name);      
    }else{
      item = wib->GetFEMBItem(1,*name);      
    }
    printf("  %-58s (address: 0x%04"PRIX32" mask: 0x%08"PRIX32")",name->c_str(),item->address,item->mask);
    std::string mode;
    if(item->mode & Item::READ){mode+="r";}
    if(item->mode & Item::WRITE){mode+="w";}
    if(item->mode & Item::ACTION){mode+="a";}
    printf(" %-3s\n",mode.c_str());

    //print description (if there is only one entry in names)
    if((names.size() == 1) && (item->user.find("description") != item->user.end())){
      std::string description = item->user.find("description")->second; 
      printf("    Description:   ");
      boost::char_separator<char> sep("\n");
      boost::tokenizer<boost::char_separator<char> > tokens(description, sep);
      boost::tokenizer<boost::char_separator<char> >::iterator it = tokens.begin();
      if(it != tokens.end()){
	printf("%s\n",(*it).c_str());
	it++;
      }
      for ( ;it != tokens.end();++it){
	printf("        %s\n",(*it).c_str());
      }       
    }
  }
}

CommandReturn::status WIBTool::WIBDevice::Names(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused args
  if(strArg.size() > 0){
    PrintNames(wib->GetNames(strArg[0]));
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}





std::string WIBTool::WIBDevice::autoComplete_WIBAddressTable(std::vector<std::string> const & line,std::string const & currentToken ,int state){
  if(line.size() > 0){
    if((line.size() > 1) && (currentToken.size() == 0)){
      return std::string("");
    }
    
    static std::vector<std::string> registerName;
    static size_t iRegister;
    
    //Reload lists if we are just starting out at state == 0
    if(state == 0){
      std::string partialNameRegex(currentToken);
      partialNameRegex+="*";
      registerName = wib->GetNames(partialNameRegex);
      iRegister = 0;
    }else{
      iRegister++;
    }

    for(;iRegister < registerName.size();iRegister++){
      //Do the string compare and make sure the token is found a position 0 (the start)
      if(registerName[iRegister].find(currentToken) == 0){	
	return registerName[iRegister];
      }
    }
  }
  return std::string("");
}

CommandReturn::status WIBTool::WIBDevice::FEMBRead(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(intArg.size() == 2){
    //check for FEMB 
    if((0 >= intArg[0]) && (4 < intArg[0])){
      printf("Bad FEMB#: %"PRIu64"\n",intArg[0]);
      return CommandReturn::OK;
    }
    //Check if the argument is a numerical address or string
    if(isdigit(strArg[1][0])){
      //numeric
      uint32_t val = wib->ReadFEMB(intArg[0],intArg[1]);
      printf("FEMB%1"PRId64":0x%04"PRIX64": 0x%08"PRIX32"\n",intArg[0],intArg[1],val);
    }else{
      //string
      uint32_t val = wib->ReadFEMB(intArg[0],strArg[1]);
      printf("FEMB%1"PRId64":%s: 0x%08"PRIX32"\n",intArg[0],strArg[1].c_str(),val);      
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

CommandReturn::status WIBTool::WIBDevice::FEMBWrite(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(intArg.size() == 3){
    //check for FEMB 
    if((0 >= intArg[0]) && (4 < intArg[0])){
      printf("Bad FEMB#: %"PRId64"\n",intArg[0]);
      return CommandReturn::OK;
    }

    //Check if the argument is a numerical address or string
    if(isdigit(strArg[1][0])){      
      //numeric
      wib->WriteFEMB(intArg[0],intArg[1],intArg[2]);
      printf("FEMB%1"PRId64":0x%04"PRIX64": 0x%08"PRIX64"\n",intArg[0],intArg[1],intArg[2]);
    }else{
      //string
      wib->WriteFEMB(intArg[0],strArg[1],intArg[2]);
      printf("FEMB%1"PRId64":%s: 0x%08"PRIX64"\n",intArg[0],strArg[1].c_str(),intArg[2]);      
    }
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

CommandReturn::status WIBTool::WIBDevice::FEMBNames(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused args
  if(strArg.size() > 0){
    PrintNames(wib->GetFEMBNames(strArg[0]),false);
    return CommandReturn::OK;
  }
  return CommandReturn::BAD_ARGS;
}

CommandReturn::status WIBTool::WIBDevice::Addresses(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
  CommandReturn::status ret;
  switch (intArg.size()){
    
  case 2:
    //get a range of addresses
    PrintNames(wib->GetAddresses(intArg[0],intArg[1]));
    ret = CommandReturn::OK;
    break;
    
  case 1:
    //Get one address
    PrintNames(wib->GetAddresses(intArg[0],intArg[0]+1));
    ret = CommandReturn::OK;
    break;
  case 0:
  default:
    ret=CommandReturn::BAD_ARGS;
    break;
  }
  return ret;
}

CommandReturn::status WIBTool::WIBDevice::FEMBAddresses(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
  CommandReturn::status ret;
  switch (intArg.size()){
    
  case 2:
    //get a range of addresses
    PrintNames(wib->GetFEMBAddresses(intArg[0],intArg[1]),false);
    ret = CommandReturn::OK;
    break;
    
  case 1:
    //Get one address
    PrintNames(wib->GetFEMBAddresses(intArg[0],intArg[0]+1),false);
    ret = CommandReturn::OK;
    break;
  case 0:
  default:
    ret=CommandReturn::BAD_ARGS;
    break;
  }
  return ret;
}


std::string WIBTool::WIBDevice::autoComplete_FEMBNumber_FEMBAddressTable(std::vector<std::string> const & line,
									 std::string const & currentToken ,
									 int state)
{
  if(1 == line.size()){
    //Parse the FEMB number
    if(0 == currentToken.size()){
      static char FEMBNumber;
      if(state == 0){
	FEMBNumber = '1';
      }else {
	switch (FEMBNumber){
	case '1':
	  FEMBNumber = '2';
	  break;
	case '2':
	  FEMBNumber = '3';
	  break;
	case '3':
	  FEMBNumber = '4';
	  break;
	default:
	  FEMBNumber = 0;
	  break;
	}
      }
      if(FEMBNumber != 0){
	return std::string(1,FEMBNumber);     
      }
    }
  }else if((2 == line.size()) && (currentToken.size() == line[1].size())){
    //We've now parsed an FEMB number, but to break into the next token, we need to type a space. 
    //Since the user is expecting the tab completion to do this, we do it with the following command
    rl_insert_text(" ");
  }else if(2 <= line.size() ){
    //    return autoComplete_FEMBAddressTable(line,currentToken,state);
    static std::vector<std::string> registerName;
    static size_t iRegister;
    
    //Reload lists if we are just starting out at state == 0
    if(state == 0){
      std::string partialNameRegex(currentToken);
      partialNameRegex+="*";
      registerName = wib->GetFEMBNames(partialNameRegex);
      iRegister = 0;
    }else{
      iRegister++;
    }

    for(;iRegister < registerName.size();iRegister++){
      //Do the string compare and make sure the token is found a position 0 (the start)
      if(registerName[iRegister].find(currentToken) == 0){	
	return registerName[iRegister];
      }
    }
  }
  return std::string("");
}

std::string WIBTool::WIBDevice::autoComplete_FEMBAddressTable(std::vector<std::string> const & line,std::string const & currentToken ,int state){
  if(1 >= line.size()){
    static std::vector<std::string> registerName;
    static size_t iRegister;
    
    //Reload lists if we are just starting out at state == 0
    if(state == 0){
      std::string partialNameRegex(currentToken);
      partialNameRegex+="*";
      registerName = wib->GetFEMBNames(partialNameRegex);
      iRegister = 0;
    }else{
      iRegister++;
    }

    for(;iRegister < registerName.size();iRegister++){
      //Do the string compare and make sure the token is found a position 0 (the start)
      if(registerName[iRegister].find(currentToken) == 0){	
	return registerName[iRegister];
      }
    }
  }
  return std::string("");
}



std::string WIBTool::WIBDevice::autoComplete_WIBTables(std::vector<std::string> const & line,std::string const & currentToken ,int state){
  if(1 == line.size()){
    //Parse the status number
    if(0 == currentToken.size()){
    }
  }else if((2 == line.size()) && (currentToken.size() == line[1].size())){
    //We've now parsed an FEMB number, but to break into the next token, we need to type a space. 
    //Since the user is expecting the tab completion to do this, we do it with the following command
    rl_insert_text(" ");
  }else if(2 <= line.size() ){

    static std::vector<std::string> tableName;
    static size_t iTable;
    
    //Reload lists if we are just starting out at state == 0
    if(state == 0){
      std::string partialNameRegex(currentToken);
      partialNameRegex+="*";
      tableName = wib->GetTableNames(partialNameRegex);
      iTable = 0;
    }else{
      iTable++;
    }

    for(;iTable < tableName.size();iTable++){
      //Do the string compare and make sure the token is found a position 0 (the start)
      if(tableName[iTable].find(currentToken) == 0){	
	return tableName[iTable];
      }
    }
  }
  return std::string("");
}


CommandReturn::status WIBTool::WIBDevice::Read_QSFP_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
  if((intArg.size() == 0) || (intArg.size() > 2)){
    return CommandReturn::BAD_ARGS;
  }

  uint8_t byte_count = 1;
  uint16_t address = 0;
  switch (intArg.size()){
  case 2:    
    if(intArg[1] > 4){
      printf("Byte count %"PRIu64" is too big, using %u\n",intArg[1],byte_count);
    }else{
      byte_count = intArg[1];
    }
  case 1:
    if (intArg[0] & (~0xFFFF) ){
      //Check for any bits outside of the 16 bits are set
      printf("Address out of range(16bit)\n");
      return CommandReturn::BAD_ARGS;      
    }
    address = intArg[0];
  }
  uint32_t val = wib->ReadQSFP(address,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}


CommandReturn::status WIBTool::WIBDevice::Write_QSFP_I2C(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg; // to make compiler not complain about unused args
  if((intArg.size() < 2) || (intArg.size() > 3)){
    return CommandReturn::BAD_ARGS;
  }

  uint8_t byte_count = 1;
  uint16_t address = 0;
  uint32_t val = 0;
  switch (intArg.size()){
  case 3:
    //byte count
    if(intArg[2] > 4){
      printf("Byte count %"PRIu64" is too big, using %u\n",intArg[2],byte_count);
    }else{
      byte_count = intArg[2];
    }
  case 2:    

    //value to write
    if(intArg[1] & (~0xFFFFFFFF)){
      //Check for any bits outside of the 32 bits are set
      printf("Value out of range(32bit)\n");
      return CommandReturn::BAD_ARGS;      
    }
    val = intArg[1];

    //addresss
    if (intArg[0] & (~0xFFFF) ){
      //Check for any bits outside of the 16 bits are set
      printf("Address out of range(16bit)\n");
      return CommandReturn::BAD_ARGS;      
    }
    address = intArg[0];
  }
  wib->WriteQSFP(address,val,byte_count);
  printf("I2C:%"PRIX16": 0x%0*X\n",address,2*byte_count,val);      
  return CommandReturn::OK;
}
