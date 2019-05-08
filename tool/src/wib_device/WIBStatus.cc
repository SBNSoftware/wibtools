#include <wib_device/WIBStatus.hh>



void BUTool::WIBStatus::ProcessFEMB(uint8_t iFEMB,std::string const & singleTable){  
  //Append this FEMB to the name
  std::string FEMBName("FEMB");        
  FEMBName.append(1,'0' +iFEMB);
  FEMBName.append(1,'.');

  //Build FEMB tables
  std::vector<std::string> FEMBNames = wib->GetFEMBNames("*");  
  //process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = FEMBNames.begin();
      itName != FEMBNames.end();
      itName++){
    //Get the list of parameters for this node
    Item const * item = wib->GetFEMBItem(iFEMB,*itName);
    boost::unordered_map<std::string,std::string> parameters = item->user;
    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){	
      //Check for an enabled argument
      boost::unordered_map<std::string,std::string>::iterator itEnabled = parameters.find("Enabled");
      if(itEnabled != parameters.end()){
	if((itEnabled->second)[0] == '!'){
	  //We hvae the negative of our normal control
	  itEnabled->second = (itEnabled->second).substr(1); //Strip the '!'
	  itEnabled->second = (wib->ReadFEMB(iFEMB,itEnabled->second) == 0x0) ? "1" : "0";	  
	}else{
	  //Change the value to 0 or 1 depending on if it should be enabled
	  itEnabled->second = (wib->ReadFEMB(iFEMB,itEnabled->second) == 0x0) ? "0" : "1";
	}
      }
      
      //Check for an table name
      boost::unordered_map<std::string,std::string>::iterator itTable = parameters.find("Table");
      
      std::string tableName = itTable->second;
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
	tables[tableName].Add(FEMBName + *itName,
			      wib->ReadFEMB(iFEMB,*itName),
			      uint32_t(item->mask >> item->offset),
			      parameters);
      }
    }
  }
}



void BUTool::WIBStatus::Process(std::string const & singleTable){  
  //Build WIB tables
  std::vector<std::string> WIBNames = wib->GetNames("*");  
  //process all the nodes and build table structure
  for(std::vector<std::string>::iterator itName = WIBNames.begin();
      itName != WIBNames.end();
      itName++){
    //Get the list of parameters for this node
    Item const * item = wib->GetItem(*itName);
    boost::unordered_map<std::string,std::string> parameters = item->user;
    //Look for a Status parameter
    if(parameters.find("Status") != parameters.end()){	
      //Check for an enabled argument
      boost::unordered_map<std::string,std::string>::iterator itEnabled = parameters.find("Enabled");
      if(itEnabled != parameters.end()){
	if((itEnabled->second)[0] == '!'){
	  //We hvae the negative of our normal control
	  itEnabled->second = (itEnabled->second).substr(1); //Strip the '!'
	  itEnabled->second = (wib->Read(itEnabled->second) == 0x0) ? "1" : "0";	  
	}else{
	  //Change the value to 0 or 1 depending on if it should be enabled
	  itEnabled->second = (wib->Read(itEnabled->second) == 0x0) ? "0" : "1";
	}
      }
      
      //Check for an table name
      boost::unordered_map<std::string,std::string>::iterator itTable = parameters.find("Table");
      
      std::string tableName = itTable->second;
      //Add this Address to our Tables if it matches our singleTable option, or we are looking at all tables
      if( singleTable.empty() || TableNameCompare(tableName,singleTable)){
	tables[tableName].Add(*itName,
			      wib->Read(*itName),
			      uint32_t(item->mask >> item->offset),
			      parameters);
      }
    }
  }  



  //Build FEMB tables
  for(uint8_t iFEMB = 1; iFEMB <= 4;iFEMB++){    
    std::string regName("POWER.ENABLE.FEMB");        
    regName.append(1,'0' +iFEMB);
    //Only look at FEMBs that are powered up
    if(0x1F == wib->Read(regName)){
      //Powered up, add to tables
      ProcessFEMB(iFEMB,singleTable);
    }
  }
}
