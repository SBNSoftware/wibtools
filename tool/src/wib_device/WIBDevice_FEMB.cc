#include <wib_device/WIBDevice.hh>
#include <helpers/parseHelpers.hh>
#include <iostream>
#include <bitset>

//#include <WIB/FE_ASIC_reg_mapping.hh>
//#include <WIB/ADC_ASIC_reg_mapping.hh>
//#include <WIB/ASIC_reg_mapping.hh>

CommandReturn::status WIBTool::WIBDevice::PowerFEMB(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  if(2 != intArg.size()){
    return CommandReturn::BAD_ARGS;
  }

  //Get FEMBs to process
  std::vector<size_t> FEMBs = parseList(strArg[0]);
  
  //Get power state
  for(size_t iFEMB = 0; iFEMB < FEMBs.size();iFEMB++){      
    wib->FEMBPower(FEMBs[iFEMB],intArg[1]);
  }
  return CommandReturn::OK;
}


CommandReturn::status WIBTool::WIBDevice::ConfigWIBFakeData(std::vector<std::string> /*strArg*/,std::vector<uint64_t> intArg){

  if (intArg.size() < 4 || intArg.size() > 5){
    return CommandReturn::BAD_ARGS;
  }
  for (const auto arg: intArg) {
    if (arg > 1){
      return CommandReturn::BAD_ARGS;
    }
  }

  uint64_t counter = 0;
  if (intArg.size() == 5){
    counter = intArg.at(4);
  }

  wib->ConfigWIBFakeData(intArg.at(0),intArg.at(1),intArg.at(2),intArg.at(3),counter);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::SetFEMBFakeData(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused arguments

  if(strArg.size() != 3){
    return CommandReturn::BAD_ARGS;
  }
  bool mode;
  if((1 == strArg[2].size()) && (toupper(strArg[2][0]) == 'C')){
    mode = true;
  }else if((1 == strArg[2].size()) && (toupper(strArg[2][0]) == 'S')){
    mode = false;
  }else{
    printf("Unkown sample/counter mode: %s\n",strArg[2].c_str());
    return CommandReturn::BAD_ARGS;
  }

  //Get FEMBs to process
  std::vector<size_t> FEMBs = parseList(strArg[0]);

  //Get fake CDAs to process
  std::vector<size_t> CDAs = parseList(strArg[1]);

  //Setup the FEMBs/Links
  for(size_t iFEMB = 0; iFEMB < FEMBs.size();iFEMB++){    
    for(size_t iCDA = 0; iCDA < CDAs.size();iCDA++){
      wib->SetFEMBFakeCOLDATAMode(FEMBs[iFEMB],CDAs[iCDA],mode);
    }
  }

  return CommandReturn::OK;
}
CommandReturn::status WIBTool::WIBDevice::SetFEMBStreamSource(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg; // to make compiler not complain about unused arguments

  if(strArg.size() != 3){
    return CommandReturn::BAD_ARGS;
  }
  bool real;
  if((1 == strArg[2].size()) && (toupper(strArg[2][0]) == 'F')){
    real = false;
  }else if((1 == strArg[2].size()) && (toupper(strArg[2][0]) == 'R')){
    real = true;
  }else{
    printf("Unkown mode: %s\n",strArg[2].c_str());
    return CommandReturn::BAD_ARGS;
  }


  //Get FEMBs to process
  std::vector<size_t> FEMBs = parseList(strArg[0]);

  //Get Links to process
  std::vector<size_t> links = parseList(strArg[1]);

  //Setup the FEMBs/Links
  for(size_t iFEMB = 0; iFEMB < FEMBs.size();iFEMB++){    
    for(size_t iLink = 0; iLink < links.size();iLink++){
      wib->SetFEMBStreamSource(FEMBs[iFEMB],links[iLink],real);
    }
  }
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::EnableADC(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) strArg;

  if (intArg.size() != 2) return CommandReturn::BAD_ARGS;

  uint64_t iFEMB = intArg[0];
  uint64_t enable = intArg[1];

  if(iFEMB > 4 || iFEMB < 1) return CommandReturn::BAD_ARGS;
  if(enable > 1) return CommandReturn::BAD_ARGS;

  wib->EnableADC(iFEMB,enable);

  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::FEMBSource(std::vector<std::string> strArg,std::vector<uint64_t> intArg){
  (void) intArg;
  //Get Links to process
  if(strArg.size() != 2) return CommandReturn::BAD_ARGS;
  std::vector<size_t> FEMBs = parseList(strArg[0]);
  

  for(size_t iFEMB = 0; iFEMB < FEMBs.size();iFEMB++){    
    if(FEMBs[iFEMB] > 4 || FEMBs[iFEMB] < 1) return CommandReturn::BAD_ARGS; 
  }
  uint64_t real;
  if((1 == strArg[1].size()) && (toupper(strArg[1][0]) == 'F')){
    real = 0xF;
  }else if((1 == strArg[1].size()) && (toupper(strArg[1][0]) == 'R')){
    real = 0x0;
  }else{
    printf("Unkown mode: %s\n",strArg[1].c_str());
    return CommandReturn::BAD_ARGS;
  }

  //Setup the FEMBs/Links
  for(size_t iFEMB = 0; iFEMB < FEMBs.size();iFEMB++){    
    wib->SourceFEMB(FEMBs[iFEMB],real);
  }
  return CommandReturn::OK;
}


CommandReturn::status WIBTool::WIBDevice::ConfigFEMB(std::vector<std::string> strArg, std::vector<uint64_t> intArg)
{
  (void) strArg;
  const size_t nArgs = intArg.size();

  if(nArgs < 11){
    std::cout << "Error: Not enough args to configFEMB" << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  
  uint8_t iFEMB = intArg[0];
  std::vector<uint32_t> fe_config;

  if(intArg[1] > 3){
    std::cout << "Error: Invalid Gain Setting --  " << intArg[1] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[2] > 3){
    std::cout << "Error: Invalid Shaping Time Setting --  " << intArg[2] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[3] > 2){
    std::cout << "Error: Invalid High Baseline Setting --  " << intArg[3] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[4] > 1){
    std::cout << "Error: Invalid High Leakage Setting --  " << intArg[4] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[5] > 1){
    std::cout << "Error: Invalid Leakage x10 Setting --  " << intArg[5] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[6] > 1){
    std::cout << "Error: Invalid AC Coupling Setting --  " << intArg[6] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[7] > 1){
    std::cout << "Error: Invalid Buffer Setting --  " << intArg[7] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[8] > 1){
    std::cout << "Error: Invalid External Clock Setting --  " << intArg[9] << std::endl;
    return CommandReturn::BAD_ARGS;
  }

  for(uint8_t i = 1; i < 9; ++i){
    fe_config.push_back(intArg[i]);
  }

  if(intArg[9] > 2){
    std::cout << "Error: Invalid pulser mode setting --  " << intArg[8] << std::endl;
    return CommandReturn::BAD_ARGS;
  }
  if(intArg[10] > 63){
    std::cout << "Error: Invalid pulser DAC amplitude setting --  " << intArg[9] << std::endl;
    return CommandReturn::BAD_ARGS;
  }

  uint8_t pls_mode = intArg[9];
  uint8_t pls_dac_val = intArg[10];

  std::vector<uint16_t> clk_phases;
  for(uint8_t i = 11; i < nArgs; ++i){
    const uint64_t phase = intArg[i];
    if(phase > 0xFFFF){
      std::cout << "Error: clock phase must be <= 0xFFFF, not: " << intArg[i] << std::endl;
      return CommandReturn::BAD_ARGS;
    }
    clk_phases.push_back(phase);
  }
 
  wib->ConfigFEMB(iFEMB, fe_config, clk_phases, pls_mode, pls_dac_val);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ConfigFEMBFakeData(std::vector<std::string> /*strArg*/, std::vector<uint64_t> intArg)
{
   if(intArg.size() > 3 || intArg.size() < 2){return CommandReturn::BAD_ARGS;}
   uint64_t iFEMB = intArg.at(0);
   uint64_t fake_mode = intArg.at(1);
   if(fake_mode < 1 || fake_mode > 4){return CommandReturn::BAD_ARGS;}
   uint64_t fake_word = 0;
   if(fake_mode == 1){
       if(intArg.size() < 3){return CommandReturn::BAD_ARGS;}
       fake_word = intArg.at(2);
   }
   std::vector<uint32_t> fake_samples(256);
   for (size_t iSample=0; iSample < 256; iSample++){
     fake_samples.at(iSample) = (iSample << 8) + iSample;
   }
   
   wib->ConfigFEMBFakeData(iFEMB, fake_mode, fake_word, iFEMB, fake_samples);
   return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::ConfigFEMBMode(std::vector<std::string> strArg, std::vector<uint64_t> intArg)
{
   (void) strArg;
   if(intArg.size() != 6){return CommandReturn::BAD_ARGS;}
   uint8_t iFEMB = intArg[0];
   uint32_t pls_cs = intArg[1];
   uint32_t dac_sel = intArg[2];
   uint32_t fpga_dac = intArg[3];
   uint32_t asic_dac = intArg[4];
   uint32_t mon_cs = intArg[5]; 

   wib->ConfigFEMBMode( iFEMB,  pls_cs,  dac_sel,  fpga_dac,  asic_dac,  mon_cs);
   return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::SetupFEMBExtClock(std::vector<std::string> strArg,std::vector<uint64_t> intArg)
{
  if(1 != strArg.size()){
    return CommandReturn::BAD_ARGS;
  }
  if(1 != intArg.size()){
    return CommandReturn::BAD_ARGS;
  }

  uint8_t iFEMB = intArg[0];
  wib->SetupFEMBExtClock(iFEMB);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::SetupFPGAPulser(std::vector<std::string> strArg, std::vector<uint64_t> intArg){
  (void) strArg;

  if(intArg.size() != 2){return CommandReturn::BAD_ARGS;}

  uint8_t iFEMB = intArg[0];
  uint8_t dac_val = intArg[1];
  wib->SetupFPGAPulser(iFEMB, dac_val);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::SetupASICPulserBits(std::vector<std::string> strArg, std::vector<uint64_t> intArg){
  (void) strArg;

  if(intArg.size() != 1){return CommandReturn::BAD_ARGS;}

  uint8_t iFEMB = intArg[0];
  wib->SetupASICPulserBits(iFEMB);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::SetupFEMBASICs(std::vector<std::string> strArg,std::vector<uint64_t> intArg)
{
  (void) strArg; // to make compiler not complain about unused arguments

  size_t intArgSize = intArg.size();
  if(intArgSize<1 || intArgSize>11){
    std::cout << "Error: SetupFEMBASICs requires between 1 and 11 args"<< std::endl;
    return CommandReturn::BAD_ARGS;
  }

  // Defaults
  std::vector<uint8_t> myIntArg(11);
  myIntArg[1] = 0;
  myIntArg[2] = 0;
  myIntArg[3] = 0;
  myIntArg[4] = 0;
  myIntArg[5] = 0;
  myIntArg[6] = 0;
  myIntArg[7] = 0;
  myIntArg[8] = 0;
  myIntArg[9] = 0;
  myIntArg[10] = 0;
  size_t myIntArgSize = myIntArg.size();

  for (size_t iArg=0; iArg < intArgSize; iArg++){
    myIntArg[iArg] = intArg[iArg];
  }

  for (size_t iArg=0; iArg < myIntArgSize; iArg++){
    if (iArg == 0 && (myIntArg[iArg] > 4 || myIntArg[iArg] < 1)){
        std::cout << "Error: iFEMB must be 1-4"<< std::endl;
        return CommandReturn::BAD_ARGS;
    }
    if ((iArg > 0 && iArg < 3) && myIntArg[iArg] > 3){
        std::cout << "Error: gain and shape must be 0-3"<< std::endl;
        return CommandReturn::BAD_ARGS;
    }
    if ((iArg == 3) && myIntArg[iArg] > 2){
        std::cout << "Error: baseline must be 0-2"<< std::endl;
        return CommandReturn::BAD_ARGS;
    }
    if ((iArg == 9) && myIntArg[iArg] > 2){
        std::cout << "Error: arguments 9 must be 0, 1, or 2"<< std::endl;
        return CommandReturn::BAD_ARGS;
    }
    if ((iArg == 10) && myIntArg[iArg] > 63){
        std::cout << "Error: arguments 9 must be 0-63"<< std::endl;
        return CommandReturn::BAD_ARGS;
    }
    if ((iArg > 3) && myIntArg[iArg] > 1){
        std::cout << "Error: arguments 4 through 10 must be 0 or 1 (bool)"<< std::endl;
        return CommandReturn::BAD_ARGS;
    }
  }

  //// To test out mapping classes
  //FE_ASIC_reg_mapping fe_map;
  //uint8_t sg=3;
  //uint8_t sp=3;
  //fe_map.set_board(0,0,sg,sp);
  //fe_map.print();
  //ADC_ASIC_reg_mapping adc_map;
  //uint8_t d=12;
  //uint8_t clk0=1;
  //uint8_t engr=1;
  //adc_map.set_board(d, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, clk0, 0, 0, engr, 0, 0, 0, 0);
  //adc_map.print();
  //ASIC_reg_mapping map;
  //map.set_board(fe_map,adc_map);
  //map.print();

  uint8_t iFEMB = myIntArg[0];
  uint8_t gain = myIntArg[1];
  uint8_t shape = myIntArg[2];
  bool highBaseline = myIntArg[3];
  bool highLeakage = myIntArg[4];
  bool highLeakage10 = myIntArg[5];
  bool acCoupling = myIntArg[6];
  bool buffer = myIntArg[7];
  bool useExtClock = myIntArg[8];
  uint8_t internalDACControl = myIntArg[9];
  uint8_t internalDACValue = myIntArg[10];
  uint16_t syncStatus = wib->SetupFEMBASICs(iFEMB,gain,shape,highBaseline,highLeakage,highLeakage10,acCoupling,buffer,useExtClock,internalDACControl,internalDACValue);
  std::cout << "FEMB "<< int(iFEMB) <<" ASIC sync status: " << std::bitset<16>(syncStatus) << std::endl;
  return CommandReturn::OK;
}
