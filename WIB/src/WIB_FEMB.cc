#include <WIB.hh>
#include <WIBException.hh>
#include <FE_ASIC_reg_mapping.hh>
#include <ADC_ASIC_reg_mapping.hh>
#include <ASIC_reg_mapping.hh>
#include <BNL_FE_Reg_Mapping.hh>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <fstream>
#include "trace.h"

#define sleep(x) usleep((useconds_t) x * 1e6)

/** \brief Setup FEMB in real or pulser data mode
 *
 *  Sets up iFEMB (index from 1)
 *  fe_config: list of options to configure the FE ASICs:
 *          Gain: 0,1,2,3 for 4.7, 7.8, 14, 25 mV/fC, respectively
 *          Shaping Time: 0,1,2,3 for 0.5, 1, 2, 3 us, respectively
 *          High Baseline: 0 for 200 mV, 1 for 900 mV, 2 for 200 mV on collection and 900 mV on induction
 *          High Leakage: 0 for 100 pA, 1 for 500 pA
 *          Leakage x 10: if 1, multiply leakage times 10
 *          AC Coupling : 0 for DC coupling, 1 for AC coupling (between FE and ADC)
 *          Buffer: 0 for disable and bypass, 1 for use (between FE and ADC)
 *          Use External Clock: 0 ADC use internal clock, 1 ADC use FPGA clocking (almost always want 1)
 *  clk_phases: a list of 16 bit values to try for the ADC clock phases.
 *      Tries these values until the sync check bits are all 0, and hunts 
 *        for good values if these all fail.
 *      The most significant byte is ADC_ASIC_CLK_PHASE_SELECT (register 6)
 *        while the least significant byte is ADC_ASIC_CLK_PHASE_SELECT (register 15)
 *  pls_mode: pulser mode select: 0 off, 1 FE ASIC internal pulser, 2 FPGA pulser
 *  pls_dac_val: pulser DAC value (amplitude) 
 *      6-bits in ASIC test pulse mode, 5-bits in FPGA test pulse mode
 *  start_frame_mode_sel: 1 to make data frame start the way BU WIB firmware expects
 *  start_frame_swap: 1 to reverse the start bits
 */
void WIB::ConfigFEMB(uint8_t iFEMB, 
		     std::vector<uint32_t> fe_config, 
		     std::vector<uint16_t> clk_phases,
		     uint8_t pls_mode, 
		     uint8_t pls_dac_val, 
		     uint8_t start_frame_mode_sel, 
		     uint8_t start_frame_swap){
  const std::string identification = "WIB::ConfigFEMB";  
  TLOG_INFO(identification) << "================== Starting ConfigFEMB function =====================" << TLOG_ENDL;		     

  if (iFEMB < 1 || iFEMB > 4)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMB: iFEMB should be between 1 and 4: "
            << int(iFEMB);
     e.Append(expstr.str().c_str());
     throw e;
  }
  if (pls_mode > 2)
  {
    WIBException::WIB_BAD_ARGS e;
    std::stringstream expstr;
    expstr << "ConfigFEMB: pls_dac_mode is allowed to be 0 (off), 1 (FPGA), 2 (internal), but is: "
           << int(pls_mode);
    e.Append(expstr.str().c_str());
    throw e;
  }
  if (start_frame_mode_sel > 1 || start_frame_swap > 1)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMB: start_frame_mode_sel and start_frame_swap must be 0 or 1";
     e.Append(expstr.str().c_str());
     throw e;
  }

  if(fe_config.size() != 8){

    WIBException::WIB_BAD_ARGS e;
    std::stringstream expstr;
    expstr << "Error: Expecting 9 Front End configuration options:" << std::endl <<
    "\t0: Gain" << std::endl << 
    "\t1: Shaping Time" << std::endl << 
    "\t2: High Baseline" << std::endl << 
    "\t3: High Leakage" << std::endl << 
    "\t4: Leakage x 10" << std::endl << 
    "\t5: AC Coupling" << std::endl << 
    "\t6: Buffer" << std::endl << 
    "\t7: Use External Clock" << std::endl;     
    e.Append(expstr.str().c_str());
    throw e;
  }
  else{
    TLOG_INFO(identification) << "Front End configuration options:" << TLOG_ENDL <<  
    "\t0:" << std::setw(22) << std::setfill(' ') <<  "Gain "               << fe_config[0] << TLOG_ENDL <<
    "\t1:" << std::setw(22) << std::setfill(' ') <<  "Shaping Time "       << fe_config[1] << TLOG_ENDL <<
    "\t2:" << std::setw(22) << std::setfill(' ') <<  "High Baseline "      << fe_config[2] << TLOG_ENDL <<
    "\t3:" << std::setw(22) << std::setfill(' ') <<  "High Leakage "       << fe_config[3] << TLOG_ENDL <<
    "\t4:" << std::setw(22) << std::setfill(' ') <<  "Leakage x 10 "       << fe_config[4] << TLOG_ENDL <<
    "\t5:" << std::setw(22) << std::setfill(' ') <<  "AC Coupling "        << fe_config[5] << TLOG_ENDL <<
    "\t6:" << std::setw(22) << std::setfill(' ') <<  "Buffer "             << fe_config[6] << TLOG_ENDL <<
    "\t7:" << std::setw(22) << std::setfill(' ') <<  "Use External Clock " << fe_config[7] << TLOG_ENDL; 
  }

  TLOG_INFO(identification) << "Pulser Mode: " << int(pls_mode) << " and DAC Value: " << int(pls_dac_val) << TLOG_ENDL;

  // get this register so we can leave it in the state it started in
  //uint32_t slow_control_dnd = Read("SYSTEM.SLOW_CONTROL_DND");
  //Write("SYSTEM.SLOW_CONTROL_DND",1);
  
  TLOG_INFO(identification) << "FW VERSION " << ReadFEMB(iFEMB,"VERSION_ID") << TLOG_ENDL;
  TLOG_INFO(identification) << "SYS_RESET " << ReadFEMB(iFEMB,"SYS_RESET") << TLOG_ENDL;
  
  if(ReadFEMB(iFEMB,"VERSION_ID") == ReadFEMB(iFEMB,"SYS_RESET")) { // can't read register if equal
    if(ContinueOnFEMBRegReadError){
      TLOG_INFO(identification) << "Error: Can't read registers from FEMB " << int(iFEMB) << TLOG_ENDL;
      return;
    }
    WIBException::FEMB_REG_READ_ERROR e;
    std::stringstream expstr;
    expstr << " for FEMB: " << int(iFEMB);
    e.Append(expstr.str().c_str());
    throw e;
  }
  TLOG_INFO(identification) << "FW VERSION " << std::hex << ReadFEMB(iFEMB,"VERSION_ID") << std::dec << TLOG_ENDL;

  //WriteFEMB(iFEMB, "REG_RESET", 1);
  //sleep(1);

  //WriteFEMB(iFEMB, "START_FRAME_MODE_SELECT", start_frame_mode_sel);
  //sleep(1);
  //WriteFEMB(iFEMB, "START_FRAME_SWAP", start_frame_swap);

  /* only for P1 ADC
  if(fe_config[7]){ // use external clock
    SetupFEMBExtClock(iFEMB);
  }
  sleep(0.05);
  */
  
  TLOG_INFO(identification) << "Time stamp reset" << TLOG_ENDL;
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1); // Write only
  sleep(0.1);
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
  sleep(0.1);
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
  TLOG_INFO(identification) << "here" << TLOG_ENDL;
  
  // This section was commented in the original wibtools packaged
  // Uncommented that to be compatible with the femb_config.py in the BNL CE code
  // Corresponding line numbers in the femb_config.py program are 380 - 385

  // These are all Jack's WIB addresses, need to figure out Dan's addresses for functionality
  ////Sync Time stamp /WIB
  Write(1, 0);
  Write(1, 0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, 1, 30);
  Write(1, 2);
  Write(1, 2);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(2, 1, 30);
  Write(1, 0);
  Write(1, 0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, 1, 30);
  //
  ////Reset Error /WIB
  Write(18, 0x8000);
  Write(18, 0x8000);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0x8000, 18, 30);

  //Reset SPI
  //
  sleep(0.005);
  WriteFEMB(iFEMB, "ADC_RESET", 0x1); // ADC_ASIC_RESET // Write only
  //sleep(0.005);
  sleep(0.1);
  WriteFEMB(iFEMB, "ADC_RESET", 0x1); // ADC_ASIC_RESET
  //sleep(0.005);
  sleep(0.1);
  WriteFEMB(iFEMB, "ADC_RESET", 0x1);
  sleep(0.1);
  WriteFEMB(iFEMB, "FE_RESET", 0x1); // FE_ASIC_RESET // Write only
  //sleep(0.005);
  sleep(0.1);
  WriteFEMB(iFEMB, "FE_RESET", 0x1);
  sleep(0.1);
  WriteFEMB(iFEMB, "FE_RESET", 0x1); // FE_ASIC_RESET
  sleep(0.005);
  
  TLOG_INFO(identification) << "ADC_RESET & FE_RESET done" << TLOG_ENDL;

  /* only for P1 ADC
  //Set ADC latch_loc
  uint32_t REG_LATCHLOC1_4_data = 0x04040404;
  uint32_t REG_LATCHLOC5_8_data = 0x04040404;
  
  WriteFEMB(iFEMB, "ADC_LATCH_LOC_0TO3", REG_LATCHLOC1_4_data);
  WriteFEMB(iFEMB, "ADC_LATCH_LOC_4TO7", REG_LATCHLOC5_8_data);
  */

  // Setup pulser
  uint8_t internal_daq_value = 0;
  if (pls_mode == 1) // internal, FE ASIC, 6 bits
  {
    TLOG_INFO(identification) << "************ USING INTERNAL PULSAR FOR CALIBRATION *****************" << TLOG_ENDL;
    //if (pls_dac_val >= 63)
    if (pls_dac_val > 63)
    {
      WIBException::WIB_BAD_ARGS e;
      std::stringstream expstr;
      expstr << "ConfigFEMB: pls_dac_val is 6 bits for internal DAC, must be 0-63, but is: "
             << int(pls_dac_val);
      e.Append(expstr.str().c_str());
      throw e;
    }
    internal_daq_value = pls_dac_val;
    SetupInternalPulser(iFEMB);
  }
  else if (pls_mode == 2) // external, FPGA, 6 bits
  {
    TLOG_INFO(identification) << "************ USING EXTERNAL PULSAR FOR CALIBRATION *****************" << TLOG_ENDL;
    //if (pls_dac_val >= 63)
    if (pls_dac_val > 63)
    {
      WIBException::WIB_BAD_ARGS e;
      std::stringstream expstr;
      expstr << "ConfigFEMB: pls_dac_val is 6 bits for FPGA DAC, must be 0-63, but is: "
             << int(pls_dac_val);
      e.Append(expstr.str().c_str());
      throw e;
    }
    /*internal_daq_value = pls_dac_val;*/ // Is this needed
    SetupFPGAPulser(iFEMB,pls_dac_val);
  }
  
  
  // Default test data pattern inserted is 0x123 (Accoding to Jack's document)
  // Test done using test stand at D0 shows default register value is 0
  // So This register is set to default setting here
  
  //WriteFEMB(iFEMB, "DATA_TEST_PATTERN",0x123);
  
  /*
  TLOG_INFO(identification) << "DATA TEST PATTERN : " << std::hex << ReadFEMB(iFEMB,"DATA_TEST_PATTERN") << std::dec << TLOG_ENDL;
  */
  
  // COTS phase setting
  // Default settings of phase selections for  FPGA is 0x00000000
  // Probably there is no need of this section if we are using the default setting
  
  /*WriteFEMB(iFEMB, "ADC_PHASE_FE1", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE2", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE3", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE4", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE5", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE6", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE7", 0x00000000);
  WriteFEMB(iFEMB, "ADC_PHASE_FE8", 0x00000000);*/
  
  /*
  TLOG_INFO(identification) << "ADC phase FE1 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE1") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE2 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE2") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE3 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE3") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE4 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE4") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE5 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE5") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE6 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE6") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE7 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE7") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC phase FE8 : " << ReadFEMB(iFEMB, "ADC_PHASE_FE8") << TLOG_ENDL;
  */
  
  // COTS shift setting
  // Default settings for bit shifts for ADC data of FE is 0x00000000
  // Probably there is no need of this section if we are using the default setting
  // Other possible register settings are 0xAAAAAAAA,0x55555555,0xFFFFFFFF (using information in femb_config.py in BNL CE code)
  
  /*WriteFEMB(iFEMB, "ADC_DLY_FE1",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE2",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE3",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE4",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE5",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE6",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE7",0x00000000);
  WriteFEMB(iFEMB, "ADC_DLY_FE8",0x00000000);*/
  
  /*
  TLOG_INFO(identification) << "ADC DLY FE1 : " << ReadFEMB(iFEMB, "ADC_DLY_FE1") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE2 : " << ReadFEMB(iFEMB, "ADC_DLY_FE2") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE3 : " << ReadFEMB(iFEMB, "ADC_DLY_FE3") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE4 : " << ReadFEMB(iFEMB, "ADC_DLY_FE4") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE5 : " << ReadFEMB(iFEMB, "ADC_DLY_FE5") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE6 : " << ReadFEMB(iFEMB, "ADC_DLY_FE6") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE7 : " << ReadFEMB(iFEMB, "ADC_DLY_FE7") << TLOG_ENDL;
  TLOG_INFO(identification) << "ADC DLY FE8 : " << ReadFEMB(iFEMB, "ADC_DLY_FE8") << TLOG_ENDL;
  */
  
  // default value is 0
  /*WriteFEMB(iFEMB, "FEMB_SYSTEM_CLOCK_SWITCH",0);
  WriteFEMB(iFEMB, "FEMB_SYSTEM_CLOCK_SWITCH",0);*/
  
  // default value is 1
  /*WriteFEMB(iFEMB, "ADC_DISABLE_REG",1);
  WriteFEMB(iFEMB, "ADC_DISABLE_REG",1);*/
  
  /*
  TLOG_INFO(identification) << "Value of ADC_DISABLE_REG : " << ReadFEMB(iFEMB, "ADC_DISABLE_REG") << TLOG_ENDL;
  TLOG_INFO(identification) << "Value of FEMB_SYSTEM_CLOCK_SWITCH : " << ReadFEMB(iFEMB, "FEMB_SYSTEM_CLOCK_SWITCH") << TLOG_ENDL;
  */
  
  // Setup ASICs
  TLOG_INFO(identification) << "Just before setting up FEMBASICS" << TLOG_ENDL;
  SetupFEMBASICs(iFEMB, fe_config[0], fe_config[1], fe_config[2], fe_config[3], fe_config[4], fe_config[5], fe_config[6], fe_config[7], pls_mode, internal_daq_value); 
  TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " Successful SPI config" << TLOG_ENDL;

  /* only for P1 ADCs
  // Try to sync ADCs
  if (clk_phases.size() == 0)
  {
    clk_phases.push_back(0xFFFF);
  }
  if (!TryFEMBPhases(iFEMB,clk_phases)) {
    if(ContinueIfListOfFEMBClockPhasesDontSync){
      std::cout << "Warning: FEMB " << int(iFEMB) << " ADC FIFO not synced from expected phases, trying to hunt for phases" << std::endl;
      if (!HuntFEMBPhase(iFEMB,clk_phases.at(0))) {
        if(ContinueOnFEMBSyncError) {
            std::cout << "Error: FEMB " << int(iFEMB) << " ADC FIFO could not be synced even after hunting" << std::endl;
        }
        else {
          uint16_t adc_fifo_sync = ( ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16;
          WIBException::FEMB_ADC_SYNC_ERROR e;
          std::stringstream expstr;
          expstr << " after hunting. ";
          expstr << " FEMB: " << int(iFEMB);
          expstr << " sync: " << std::bitset<16>(adc_fifo_sync);
          expstr << " phases: ";
          expstr << std::hex << std::setfill ('0') << std::setw(2) << ReadFEMB(iFEMB,"ADC_ASIC_CLK_PHASE_SELECT");
          expstr << std::hex << std::setfill ('0') << std::setw(2) << ReadFEMB(iFEMB,"ADC_ASIC_CLK_PHASE_SELECT_2");
          e.Append(expstr.str().c_str());
          throw e;
        }
      } // if ! HuntFEMBPhase
    } // if ContinueIfListOfFEMBClockPhasesDontSync
    else // ContinueIfListOfFEMBClockPhasesDontSync
    {
      uint16_t adc_fifo_sync = ( ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16;
      WIBException::FEMB_ADC_SYNC_ERROR e;
      std::stringstream expstr;
      expstr << " after trying all in list. ";
      expstr << " FEMB: " << int(iFEMB);
      expstr << " sync: " << std::bitset<16>(adc_fifo_sync);
      expstr << " phases tried: " << std::endl;
      for (size_t iclk_phase = 0;iclk_phase < clk_phases.size();iclk_phase++)
      {
        expstr << "    "
               << std::hex << std::setfill ('0') << std::setw(4) << clk_phases[iclk_phase] << std::endl;
      }
      e.Append(expstr.str().c_str());
      throw e;
    } // else ContinueIfListOfFEMBClockPhasesDontSync
  } // if ! TryFEMBPhases
  uint16_t adc_fifo_sync = ( ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16;
  std::cout << "FEMB " << int(iFEMB) << " Final ADC FIFO sync: " << std::bitset<16>(adc_fifo_sync) << std::endl;
  std::cout << "FEMB " << int(iFEMB) << " Final Clock Phases: "
                    << std::hex << std::setfill ('0') << std::setw(2) << ReadFEMB(iFEMB,"ADC_ASIC_CLK_PHASE_SELECT")
                    << std::hex << std::setfill ('0') << std::setw(2) << ReadFEMB(iFEMB,"ADC_ASIC_CLK_PHASE_SELECT_2")
                    << std::endl;
  */


  //Initializing the COTS ADCs shift and phase variables
  uint8_t fe1_sft_RT = 0x00000000;
  uint8_t fe2_sft_RT = 0x00000000;
  uint8_t fe3_sft_RT = 0x00000000;
  uint8_t fe4_sft_RT = 0x00000000;
  uint8_t fe5_sft_RT = 0x00000000;
  uint8_t fe6_sft_RT = 0x00000000;
  uint8_t fe7_sft_RT = 0x00000000;
  uint8_t fe8_sft_RT = 0x00000000;

  uint8_t fe1_sft_CT = 0x00000000;
  uint8_t fe2_sft_CT = 0x00000000;
  uint8_t fe3_sft_CT = 0x00000000;
  uint8_t fe4_sft_CT = 0x00000000;
  uint8_t fe5_sft_CT = 0x00000000;
  uint8_t fe6_sft_CT = 0x00000000;
  uint8_t fe7_sft_CT = 0x00000000;
  uint8_t fe8_sft_CT = 0x00000000;

  uint8_t fe1_pha_RT = 0x00000000;
  uint8_t fe2_pha_RT = 0x00000000;
  uint8_t fe3_pha_RT = 0x00000000;
  uint8_t fe4_pha_RT = 0x00000000;
  uint8_t fe5_pha_RT = 0x00000000;
  uint8_t fe6_pha_RT = 0x00000000;
  uint8_t fe7_pha_RT = 0x00000000;
  uint8_t fe8_pha_RT = 0x00000000;

  uint8_t fe1_pha_CT = 0x00000000;
  uint8_t fe2_pha_CT = 0x00000000;
  uint8_t fe3_pha_CT = 0x00000000;
  uint8_t fe4_pha_CT = 0x00000000;
  uint8_t fe5_pha_CT = 0x00000000;
  uint8_t fe6_pha_CT = 0x00000000;
  uint8_t fe7_pha_CT = 0x00000000;
  uint8_t fe8_pha_CT = 0x00000000;

  //writing on FEMB

  bool RT=false; // use CT sinse they are the same

  if(RT)
    {
      TLOG_INFO(identification) << "Configuring COTS ADCS for RT" << TLOG_ENDL;
      WriteFEMB(iFEMB, 21, fe1_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe1_sft_RT, 21, iFEMB, 30);
      WriteFEMB(iFEMB, 29, fe1_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe1_pha_RT, 29, iFEMB, 30);
      WriteFEMB(iFEMB, 22, fe2_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe2_sft_RT, 22, iFEMB, 30);
      WriteFEMB(iFEMB, 30, fe2_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe2_pha_RT, 30, iFEMB, 30);
      WriteFEMB(iFEMB, 23, fe3_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe3_sft_RT, 23, iFEMB, 30);
      WriteFEMB(iFEMB, 31, fe3_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe3_pha_RT, 31, iFEMB, 30);
      WriteFEMB(iFEMB, 24, fe4_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe4_sft_RT, 24, iFEMB, 30);
      WriteFEMB(iFEMB, 32, fe4_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe4_pha_RT, 32, iFEMB, 30);
      WriteFEMB(iFEMB, 25, fe5_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe5_sft_RT, 25, iFEMB, 30);
      WriteFEMB(iFEMB, 33, fe5_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe5_pha_RT, 33, iFEMB, 30);
      WriteFEMB(iFEMB, 26, fe6_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe6_sft_RT, 26, iFEMB, 30);
      WriteFEMB(iFEMB, 34, fe6_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe6_pha_RT, 34, iFEMB, 30);
      WriteFEMB(iFEMB, 27, fe7_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe7_sft_RT, 27, iFEMB, 30);
      WriteFEMB(iFEMB, 35, fe7_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe7_pha_RT, 35, iFEMB, 30);
      WriteFEMB(iFEMB, 28, fe8_sft_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe8_sft_RT, 28, iFEMB, 30);
      WriteFEMB(iFEMB, 36, fe8_pha_RT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe8_pha_RT, 36, iFEMB, 30);
    } 
  else
    {
      TLOG_INFO(identification) << "Configuring COTS ADCS for CT" << TLOG_ENDL;
      WriteFEMB(iFEMB, 21, fe1_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe1_sft_CT, 21, iFEMB, 30);
      WriteFEMB(iFEMB, 29, fe1_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe1_pha_CT, 29, iFEMB, 30);
      WriteFEMB(iFEMB, 22, fe2_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe2_sft_CT, 22, iFEMB, 30);
      WriteFEMB(iFEMB, 30, fe2_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe2_pha_CT, 30, iFEMB, 30);
      WriteFEMB(iFEMB, 23, fe3_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe3_sft_CT, 23, iFEMB, 30);
      WriteFEMB(iFEMB, 31, fe3_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe3_pha_CT, 31, iFEMB, 30);
      WriteFEMB(iFEMB, 24, fe4_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe4_sft_CT, 24, iFEMB, 30);
      WriteFEMB(iFEMB, 32, fe4_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe4_pha_CT, 32, iFEMB, 30);
      WriteFEMB(iFEMB, 25, fe5_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe5_sft_CT, 25, iFEMB, 30);
      WriteFEMB(iFEMB, 33, fe5_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe5_pha_CT, 33, iFEMB, 30);
      WriteFEMB(iFEMB, 26, fe6_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe6_sft_CT, 26, iFEMB, 30);
      WriteFEMB(iFEMB, 34, fe6_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe6_pha_CT, 34, iFEMB, 30);
      WriteFEMB(iFEMB, 27, fe7_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe7_sft_CT, 27, iFEMB, 30);
      WriteFEMB(iFEMB, 35, fe7_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe7_pha_CT, 35, iFEMB, 30);
      WriteFEMB(iFEMB, 28, fe8_sft_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe8_sft_CT, 28, iFEMB, 30);
      WriteFEMB(iFEMB, 36, fe8_pha_CT);
      if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(fe8_pha_CT, 36, iFEMB, 30);
    }

  // This section wasn't in the original wibtools package
  // included by looking into the BNL CE code
  // See line 401 in config_femb.py function
  // and line 225 - 229
  // Done by Varuna 03/15/2023
  
  WriteFEMB(iFEMB, "FEMB_SYSTEM_CLOCK_SWITCH",0);
  WriteFEMB(iFEMB, "FEMB_SYSTEM_CLOCK_SWITCH",0);
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "FEMB_SYSTEM_CLOCK_SWITCH", iFEMB, 30);
  
  WriteFEMB(iFEMB, "ADC_DISABLE_REG",0);
  WriteFEMB(iFEMB, "ADC_DISABLE_REG",0);
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "ADC_DISABLE_REG", iFEMB, 30);
  
  sleep(0.02);
  
  WriteFEMB(iFEMB, "ADC_DISABLE_REG",1);
  WriteFEMB(iFEMB, "ADC_DISABLE_REG",1);
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(1, "ADC_DISABLE_REG", iFEMB, 30);
  
  sleep(0.02);
  
  // End of the most latest comment
  
  
  //time stamp reset
  TLOG_INFO(identification) << "Just before time stamp reset" << TLOG_ENDL;
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
  sleep(0.1);
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
  sleep(0.1);
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
 
  // These are all Jack's WIB addresses, need to figure out Dan's addresses for functionality
  ////Sync Time stamp /WIB
  Write(1, 0);
  Write(1, 0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, 1, 30);
  Write(1, 2);
  Write(1, 2);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(2, 1, 30);
  Write(1, 0);
  Write(1, 0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, 1, 30);
  //
  ////Reset Error /WIB
  Write(18, 0x8000);
  Write(18, 0x8000);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0x8000, 18, 30);
  
  FEMB_UDPACQ(iFEMB-1);

  //Write("SYSTEM.SLOW_CONTROL_DND",slow_control_dnd);
  
  //TLOG_INFO(identification) << "Configured the wib successfully" << TLOG_ENDL;
  TLOG_INFO(identification) << "Configured the FEMB successfully" << TLOG_ENDL;
}

/** \brief Setup FEMB in fake data mode
 *
 *  Sets up iFEMB (index from 1) in fake data mode
 *  fake_mode: 0 for real data, 1 for fake word, 2 for fake waveform, 3 for channel indicator (FEMB, chip, channel),
 *          4 for channel indicator (counter, chip, channel)
 *  fake_word: 12 bit wrd to use when in fake word mode
 *  femb_number: femb number to use in fake_mode 3, 4 bits
 *  fake_samples: vector of samples to use in fake_mode 2
 */
void WIB::ConfigFEMBFakeData(uint8_t iFEMB, uint8_t fake_mode, uint32_t fake_word, uint8_t femb_number, 
            std::vector<uint32_t> fake_samples, uint8_t start_frame_mode_sel, uint8_t start_frame_swap){

  if (iFEMB < 1 || iFEMB > 4)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMBFakeData: iFEMB should be between 1 and 4: "
            << int(iFEMB);
     e.Append(expstr.str().c_str());
     throw e;
  }
  if (start_frame_mode_sel > 1 || start_frame_swap > 1)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMBFakeData: start_frame_mode_sel and start_frame_swap must be 0 or 1";
     e.Append(expstr.str().c_str());
     throw e;
  }
  if (fake_mode == 1 && fake_word > 0xFFF)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMBFakeData: fake_word must be only 12 bits i.e. <= 4095, is: "
            << fake_word;
     e.Append(expstr.str().c_str());
     throw e;
  }
  if (fake_mode == 2 && fake_samples.size() != 256)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMBFakeData: femb_samples must be 255 long, is: "
            << fake_samples.size();
     e.Append(expstr.str().c_str());
     throw e;
  }
  if (fake_mode == 3 && femb_number > 0xF)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMBFakeData: femb_number must be only 4 bits i.e. <= 15, is: "
            << int(femb_number);
     e.Append(expstr.str().c_str());
     throw e;
  }

  // get this register so we can leave it in the state it started in
  //uint32_t slow_control_dnd = Read("SYSTEM.SLOW_CONTROL_DND");
  //Write("SYSTEM.SLOW_CONTROL_DND",1);

  WriteFEMB(iFEMB, "REG_RESET", 1);
  sleep(1);

  WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 0);
  sleep(1);

  WriteFEMB(iFEMB, "START_FRAME_MODE_SELECT", start_frame_mode_sel);
  sleep(1);
  WriteFEMB(iFEMB, "START_FRAME_SWAP", start_frame_swap);
  sleep(0.05);

  //time stamp reset
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);

  // Now do the Fake data mode setup
  if (fake_mode == 1)
  {
    WriteFEMB(iFEMB, "DATA_TEST_PATTERN", fake_word);
  }
  if (fake_mode == 2)
  {
    // Put waveform in FEMB registers
    for (size_t iSample=0; iSample < 256; iSample++)
    {
      WriteFEMB(iFEMB,0x300+iSample,fake_samples.at(iSample));
      sleep(0.005);
    }
  }
  if (fake_mode == 3)
  {
    WriteFEMB(iFEMB, "FEMB_NUMBER", femb_number);
  }
  WriteFEMB(iFEMB, "FEMB_TST_SEL", fake_mode);

  //time stamp reset
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);
  WriteFEMB(iFEMB, "TIME_STAMP_RESET", 1);

  WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 9);

  //Write("SYSTEM.SLOW_CONTROL_DND",slow_control_dnd);
}

/** \brief Setup FEMB External Clock
 *
 *  Sets up iFEMB (index from 1) external clock parameters
 */
void WIB::SetupFEMBExtClock(uint8_t iFEMB){
   const std::string identification = "WIB::SetupFEMBExtClock";  
  //EXTERNAL CLOCK VARIABLES
  uint32_t clk_period = 5; //ns
  uint32_t clk_dis = 0; //0 --> enable, 1 disable
  uint32_t d14_rst_oft  = 0   / clk_period;
  uint32_t d14_rst_wdt  = (45  / clk_period ) ;
  uint32_t d14_rst_inv  = 1;
  uint32_t d14_read_oft = 480 / clk_period;
  uint32_t d14_read_wdt = 20  / clk_period;
  uint32_t d14_read_inv = 1;
  uint32_t d14_idxm_oft = 230 / clk_period;
  uint32_t d14_idxm_wdt = 270 / clk_period;
  uint32_t d14_idxm_inv = 0;
  uint32_t d14_idxl_oft = 480 / clk_period;
  uint32_t d14_idxl_wdt = 20  / clk_period;
  uint32_t d14_idxl_inv = 0;
  uint32_t d14_idl0_oft = 50  / clk_period;
  uint32_t d14_idl0_wdt = (190 / clk_period ) -1;
  uint32_t d14_idl1_oft = 480 / clk_period;
  uint32_t d14_idl1_wdt = 20  / clk_period;
  uint32_t d14_idl_inv  = 0;

  uint32_t d58_rst_oft  = 0   / clk_period;
  uint32_t d58_rst_wdt  = (45  / clk_period );
  uint32_t d58_rst_inv  = 1;
  uint32_t d58_read_oft = 480 / clk_period;
  uint32_t d58_read_wdt = 20  / clk_period;
  uint32_t d58_read_inv = 1;
  uint32_t d58_idxm_oft = 230 / clk_period;
  uint32_t d58_idxm_wdt = 270 / clk_period;
  uint32_t d58_idxm_inv = 0;
  uint32_t d58_idxl_oft = 480 / clk_period;
  uint32_t d58_idxl_wdt = 20  / clk_period;
  uint32_t d58_idxl_inv = 0;
  uint32_t d58_idl0_oft = 50  / clk_period;
  uint32_t d58_idl0_wdt = (190 / clk_period ) -1;
  uint32_t d58_idl1_oft = 480 / clk_period;
  uint32_t d58_idl1_wdt = 20  / clk_period;
  uint32_t d58_idl_inv  = 0;

  //external clock phase -- Version 320
  /*uint32_t d14_read_step = 7;
  uint32_t d14_read_ud   = 0;
  uint32_t d14_idxm_step = 3;
  uint32_t d14_idxm_ud   = 0;
  uint32_t d14_idxl_step = 1;
  uint32_t d14_idxl_ud   = 1;
  uint32_t d14_idl0_step = 5;
  uint32_t d14_idl0_ud   = 0;
  uint32_t d14_idl1_step = 2;
  uint32_t d14_idl1_ud   = 0;
  uint32_t d14_phase_en  = 1;

  uint32_t d58_read_step = 1;
  uint32_t d58_read_ud   = 1;
  uint32_t d58_idxm_step = 0;
  uint32_t d58_idxm_ud   = 0;
  uint32_t d58_idxl_step = 5;
  uint32_t d58_idxl_ud   = 1;
  uint32_t d58_idl0_step = 6;
  uint32_t d58_idl0_ud   = 0;
  uint32_t d58_idl1_step = 5;
  uint32_t d58_idl1_ud   = 0;
  uint32_t d58_phase_en  = 1;*/
  //Version 323
  uint32_t d14_read_step = 11;
  uint32_t d14_read_ud   = 0;
  uint32_t d14_idxm_step = 9;
  uint32_t d14_idxm_ud   = 0;
  uint32_t d14_idxl_step = 7;
  uint32_t d14_idxl_ud   = 0;
  uint32_t d14_idl0_step = 12;
  uint32_t d14_idl0_ud   = 0;
  uint32_t d14_idl1_step = 10;
  uint32_t d14_idl1_ud   = 0;
  uint32_t d14_phase_en  = 1;

  uint32_t d58_read_step = 0;
  uint32_t d58_read_ud   = 0;
  uint32_t d58_idxm_step = 5;
  uint32_t d58_idxm_ud   = 0;
  uint32_t d58_idxl_step = 4;
  uint32_t d58_idxl_ud   = 1;
  uint32_t d58_idl0_step = 3;
  uint32_t d58_idl0_ud   = 0;
  uint32_t d58_idl1_step = 4;
  uint32_t d58_idl1_ud   = 0;
  uint32_t d58_phase_en  = 1;

  //END EXTERNAL CLOCK VARIABLES

  //config timing
  uint32_t d14_inv = (d14_rst_inv<<0) + (d14_read_inv<<1)+ (d14_idxm_inv<<2)+ (d14_idxl_inv<<3)+ (d14_idl_inv<<4);
  uint32_t d58_inv = (d58_rst_inv<<0) + (d58_read_inv<<1)+ (d58_idxm_inv<<2)+ (d58_idxl_inv<<3)+ (d58_idl_inv<<4);
  uint32_t d_inv = d58_inv + ( d14_inv<<5);

  uint32_t addr_data;

  addr_data = clk_dis + (d_inv << 16);
  WriteFEMB(iFEMB, 21, addr_data);

  addr_data = d58_rst_oft + (d14_rst_oft << 16);
  WriteFEMB(iFEMB, 22, addr_data);

  addr_data = d58_rst_wdt + (d14_rst_wdt << 16);
  WriteFEMB(iFEMB, 23, addr_data);

  addr_data = d58_read_oft + (d14_read_oft << 16);
  WriteFEMB(iFEMB, 24, addr_data);

  addr_data = d58_read_wdt + (d14_read_wdt << 16);
  WriteFEMB(iFEMB, 25, addr_data);

  addr_data = d58_idxm_oft + (d14_idxm_oft << 16);
  WriteFEMB(iFEMB, 26, addr_data);

  addr_data = d58_idxm_wdt + (d14_idxm_wdt << 16);
  WriteFEMB(iFEMB, 27, addr_data);

  addr_data = d58_idxl_oft + (d14_idxl_oft << 16);
  WriteFEMB(iFEMB, 28, addr_data);

  addr_data = d58_idxl_wdt + (d14_idxl_wdt << 16);
  WriteFEMB(iFEMB, 29, addr_data);

  addr_data = d58_idl0_oft + (d14_idl0_oft << 16);
  WriteFEMB(iFEMB, 30, addr_data);

  addr_data = d58_idl0_wdt + (d14_idl0_wdt << 16);
  WriteFEMB(iFEMB, 31, addr_data);

  addr_data = d58_idl1_oft + (d14_idl1_oft << 16);
  WriteFEMB(iFEMB, 32, addr_data);

  addr_data = d58_idl1_wdt + (d14_idl1_wdt << 16);
  WriteFEMB(iFEMB, 33, addr_data);

  //config phase 
  for(size_t i=0; i<4; i++)
  {
      addr_data = d14_read_step + (d14_idxm_step <<16);
      WriteFEMB(iFEMB, 35, addr_data);

      addr_data = d14_idxl_step + (d14_idl0_step <<16);
      WriteFEMB(iFEMB, 36, addr_data);
       
      d14_phase_en = d14_phase_en ^ 1;
      uint32_t d14_ud = d14_read_ud + (d14_idxm_ud<<1) + (d14_idxl_ud<<2)+ (d14_idl0_ud<<3)+ (d14_idl1_ud<<4) + (d14_phase_en <<15);
      addr_data = d14_idl1_step + (d14_ud<<16);
      WriteFEMB(iFEMB, 37, addr_data);

      addr_data = d58_read_step + (d58_idxm_step <<16);
      WriteFEMB(iFEMB, 38, addr_data);

      addr_data = d58_idxl_step + (d58_idl0_step <<16);
      WriteFEMB(iFEMB, 39, addr_data);
      
      d58_phase_en = d58_phase_en ^ 1;
      uint32_t d58_ud = d58_read_ud + (d58_idxm_ud<<1) + (d58_idxl_ud<<2)+ (d58_idl0_ud<<3)+ (d58_idl1_ud<<4) + (d58_phase_en <<15);
      addr_data = d58_idl1_step + (d58_ud <<16);
      WriteFEMB(iFEMB, 40, addr_data);
  } 
  sleep(0.05);
}


/** \brief Setup FEMB ASICs
 *
 *  Sets up iFEMB (index from 1) ASICs
 *
 *  registerList is a list of 71 32bit registers to program the FE and ADC ASICs
 *
 *  returns adc sync status 16 bits, one for each serial link between ADC and FPGA. There are 2 per ADC
 */
uint16_t WIB::SetupFEMBASICs(uint8_t iFEMB, std::vector<uint32_t> registerList){
  const std::string identification = "WIB::SetupFEMBASICs";
  const size_t REG_SPI_BASE = 512;
  const size_t NREGS = 36;
  
  if (registerList.size() != NREGS)
  {
    WIBException::FEMB_FIRMWARE_VERSION_MISMATCH e;
    std::stringstream expstr;
    expstr << "SetupFEMBASICs expects : "
           << NREGS
           << " argument is: "
           << registerList.size();
    e.Append(expstr.str().c_str());
    throw e;
  }

  //turn off HS data before register writes
  WriteFEMB(iFEMB, "STREAM_EN", 0 );
  sleep(2);

  for (size_t iReg=0; iReg < NREGS; iReg++)
  {
    WriteFEMB(iFEMB, REG_SPI_BASE + iReg, registerList[iReg]);
  }

  /////////////////////////////
  //run the SPI programming
  /////////////////////////////
  
  WriteFEMB(iFEMB, "ADC_ASIC_RESET", 1);
  sleep(0.01);
  WriteFEMB(iFEMB, "FE_ASIC_RESET", 1);
  sleep(0.01);
  WriteFEMB(iFEMB, "WRITE_ADC_ASIC_SPI", 1);
  sleep(0.01);
  WriteFEMB(iFEMB, "WRITE_ADC_ASIC_SPI", 1);
  sleep(0.01);
  WriteFEMB(iFEMB, "WRITE_FE_ASIC_SPI", 1);
  sleep(0.01);
  WriteFEMB(iFEMB, "WRITE_FE_ASIC_SPI", 1);
  sleep(0.01);

  //uint16_t adc_sync_status = (uint16_t) ReadFEMB(iFEMB, "ADC_ASIC_SYNC_STATUS");
  /////////////////////////////
  /////////////////////////////

  //turn HS link back on
  sleep(2);
  WriteFEMB(iFEMB, "STREAM_EN", 1 );

  return 1;
}

/** \brief Setup FEMB ASICs
 *
 *  Sets up iFEMB (index from 1) ASICs
 *
 *  gain: 0,1,2,3 for 4.7, 7.8, 14, 25 mV/fC, respectively
 *  shaping time: 0,1,2,3 for 0.5, 1, 2, 3 us, respectively
 *  highBaseline is 900mV for 1, 200mV for 0, and appropriately for each plane for 2
 *  highLeakage is 500pA for true, 100pA for false
 *  leakagex10 multiplies leakage x10 if true
 *  acCoupling: FE is AC coupled to ADC if true, DC if false
 *  buffer: FE to ADC buffer on if true, off and bypassed if false
 *  useExtClock: ADC uses external (FPGA) clock if true, internal if false
 *  internalDACControl: 0 for disabled, 1 for internal FE ASIC pulser, 2 for external FPGA pulser
 *  internalDACValue: 6 bit value for amplitude to use with internal pulser
 *
 *  returns adc sync status 16 bits, one for each serial link between ADC and FPGA. There are 2 per ADC
 */
uint16_t WIB::SetupFEMBASICs(uint8_t iFEMB, uint8_t gain, uint8_t shape, uint8_t highBaseline, 
                        bool highLeakage, bool leakagex10, bool acCoupling, bool buffer, bool useExtClock, 
                        uint8_t internalDACControl, uint8_t internalDACValue){
   const std::string identification = "WIB::SetupFEMBASICs";
  (void) buffer; // to make compiler not complain about unused arguments

  if (gain > 3) 
  {
       WIBException::WIB_BAD_ARGS e;
       std::stringstream expstr;
       expstr << "gain should be between 0 and 3, but is: "
              << int(gain);
       e.Append(expstr.str().c_str());
       throw e;
  }
  if (shape > 3) 
  {
       WIBException::WIB_BAD_ARGS e;
       std::stringstream expstr;
       expstr << "shape should be between 0 and 3, but is: "
              << int(shape);
       e.Append(expstr.str().c_str());
       throw e;
  }

  const size_t REG_SPI_BASE_WRITE = 0x200; // 512
  const size_t REG_SPI_BASE_READ = 0x250; // 592
  // for COTS ADCs first 36 registers are used for the FE ASICs

  bool bypassOutputBuffer=true; // if false might blow up SBND
  bool useOutputMonitor=false; // if true might blow up SBND
  bool useCh16HighPassFilter=false;
  bool monitorBandgapNotTemp=false;
  bool monitorTempBandgapNotSignal=false;
  bool useTestCapacitance = (bool) internalDACControl;
  
  //////////////// Cross section provided raw fcl paramters to configure ASICS ////////////
  
  TLOG_INFO(identification) << "Raw gain from fcl : " << int(gain) << TLOG_ENDL;
  TLOG_INFO(identification) << "Raw shape from fcl : " << int(shape) << TLOG_ENDL;
  TLOG_INFO(identification) << "Raw high-baseline from fcl : " << int(highBaseline) << TLOG_ENDL;
  TLOG_INFO(identification) << "Raw high-leakage current from fcl : " << highLeakage << TLOG_ENDL;
  TLOG_INFO(identification) << "Raw use buffer from fcl : " << bypassOutputBuffer << TLOG_ENDL;
  
  /////////////// End of most recent comment /////////////////////////////////////////////

  // Flip bits of gain
  if (gain == 0x1) gain = 0x2;
  else if (gain== 0x2) gain = 0x1;

  // Shape
  if (shape == 0x0) shape = 0x2; // 0.5 us
  else if (shape == 0x1) shape = 0x0; // 1 us
  else if (shape == 0x2) shape = 0x3; // 2 us
  else if (shape == 0x3) shape = 0x1; // 3 us
  
  //////////////// Cross section provided raw fcl paramters to configure ASICS ////////////
  
  TLOG_INFO(identification) << "Changed gain from fcl : " << int(gain) << TLOG_ENDL;
  TLOG_INFO(identification) << "Changed shape from fcl : " << int(shape) << TLOG_ENDL;
  TLOG_INFO(identification) << "Changed high-baseline from fcl : " << int(highBaseline) << TLOG_ENDL;
  TLOG_INFO(identification) << "Changed high-leakage current from fcl : " << !highLeakage << TLOG_ENDL;
  TLOG_INFO(identification) << "Changed use buffer from fcl : " << !bypassOutputBuffer << TLOG_ENDL;
  
  /////////////// End of most recent comment /////////////////////////////////////////////
  
  FE_ASIC_reg_mapping fe_map;
  if (highBaseline > 1)
  {
    // Set them all to high baseline
    fe_map.set_board(useTestCapacitance,0,gain,shape,
                      useOutputMonitor,1,!highLeakage,
                      monitorBandgapNotTemp,monitorTempBandgapNotSignal,useCh16HighPassFilter,
                      leakagex10,acCoupling,internalDACControl,internalDACValue
                  );
    // Now just set collection channels to low baseline
    fe_map.set_collection_baseline(1); // !bypassOutputBuffer
  }
  else
  {
    fe_map.set_board(useTestCapacitance,!highBaseline,gain,shape,
                      useOutputMonitor,1,!highLeakage,
                      monitorBandgapNotTemp,monitorTempBandgapNotSignal,useCh16HighPassFilter,
                      leakagex10,acCoupling,internalDACControl,internalDACValue
                  );
  }
  /*
  ADC_ASIC_reg_mapping adc_map;
  uint8_t offsetCurrentValue=0;
  bool pcsr=0;
  bool pdsr=0;
  bool adcSleep=0;
  bool useADCTestInput=0;
  bool f4=0;
  bool f5=0;
  bool lsbCurrentStearingPartialNotFull=0;
  bool clk0=0;
  if (useExtClock) clk0=1;
  bool clk1=0;
  bool freq=0;
  bool enableOffsetCurrent=0;
  bool f0=0;
  bool f1=0;
  bool f2=0;
  bool f3=0;
  adc_map.set_board(offsetCurrentValue, pcsr, pdsr, 
                    adcSleep, useADCTestInput, f4, f5,
                    lsbCurrentStearingPartialNotFull,0,0,
                    0,0,0,
                    clk0,clk1,freq,
                    enableOffsetCurrent,f0,f1,
                    f2,f3);
  */
  ASIC_reg_mapping map;
  map.set_board(fe_map);
  map.get_regs();
  const std::vector<uint32_t> regs = map.get_regs();
  const size_t nRegs = regs.size();

  //uint16_t adc_sync_status = 0xFFFF;

  for(unsigned iSPIWrite=0; iSPIWrite < 2; iSPIWrite++)
  {
    //TLOG_INFO(identification) << "Before PRBS_EN Register value : " << int(ReadFEMB(iFEMB,"PRBS_EN")) << " CNT_EN Register value : " << int(ReadFEMB(iFEMB,"CNT_EN")) << TLOG_ENDL; // check remaining two bits before writting to the desired address
    
    //WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 0 ); // Turn off STREAM_EN and ADC_DATA_EN (in the original code)
    //WriteFEMB(iFEMB,0x09,(ReadFEMB(iFEMB,0x09)&0xFFFFFFF6)); // solution to register not found issue
    WriteFEMB(iFEMB,0x09,0); // Copied from BNL_CE code
    if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, 0x09, iFEMB, 30);
    //TLOG_INFO(identification) << "STREAM_EN Register value : " << int(ReadFEMB(iFEMB,"STREAM_EN")) << " ADC_DATA_EN Register value : " << int(ReadFEMB(iFEMB,"ADC_DATA_EN")) << TLOG_ENDL; // Confirms two registers are properly written (code was run and tested)
    
    //TLOG_INFO(identification) << "After PRBS_EN Register value : " << int(ReadFEMB(iFEMB,"PRBS_EN")) << " CNT_EN Register value : " << int(ReadFEMB(iFEMB,"CNT_EN")) << TLOG_ENDL; // check remaining two bits after writting to the desired addresss
    
    sleep(0.1);
  
    TLOG_INFO(identification) << "ASIC SPI Write Registers..." << TLOG_ENDL;
    TLOG_INFO(identification) << "======== Number of registers : " << nRegs << TLOG_ENDL;
    for (size_t iReg=0; iReg<nRegs; iReg++)
    {
        WriteFEMB(iFEMB,REG_SPI_BASE_WRITE+iReg,regs[iReg]);
	//TLOG_INFO(identification) << "Address No. : " << iReg << " Address : " << std::hex << (REG_SPI_BASE_WRITE+iReg) << TLOG_ENDL;
	//if(iReg>69) TLOG_INFO(identification) << "============ Register number " << iReg << " Register value : " << regs[iReg] << TLOG_ENDL;
	//TLOG_INFO(identification) << "Written register value : " << regs[iReg] << TLOG_ENDL;
        sleep(0.01);
	if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(regs[iReg], REG_SPI_BASE_WRITE+iReg, iFEMB, 30);
    }
  
  
    //run the SPI programming
    sleep(0.1);
    
    //WriteFEMB(iFEMB, "WRITE_ASIC_SPI", 1); (in the original code)
    WriteFEMB(iFEMB,0x02,1);// solution to register not found issue
    sleep(0.1);
    
    //WriteFEMB(iFEMB, "WRITE_ASIC_SPI", 1); (in the original code)
    WriteFEMB(iFEMB,0x02,1);// solution to register not found issue
    sleep(0.1);
    
    WriteFEMB(iFEMB,0x02,1);
    sleep(0.1);
  
    if (iSPIWrite == 1)
    {
      // Now check readback
      bool spi_mismatch = false;
      for (unsigned iSPIRead = 0; iSPIRead < 2; iSPIRead++)
      {
        TLOG_INFO(identification)<< "ASIC SPI Readback..." << TLOG_ENDL;
        std::vector<uint32_t> regsReadback(nRegs);
        for (size_t iReg=0; iReg<nRegs; iReg++)
        {
            uint32_t regReadback = ReadFEMB(iFEMB,REG_SPI_BASE_READ+iReg);
            regsReadback[iReg] = regReadback;
            sleep(0.01);
        }
  
        bool verbose = false;
        if (verbose) TLOG_INFO(identification) << "ASIC SPI register number, write val, read val:" << TLOG_ENDL;
        spi_mismatch = false;
        for (size_t iReg=0; iReg<nRegs; iReg++)
        {
          if (verbose)
          {
            TLOG_INFO(identification) << std::dec << std::setfill (' ') << std::setw(3) << iReg 
                      << "  " 
                      << std::hex << std::setfill ('0') << std::setw(8) << regs[iReg] 
                      << "  "
                      << std::hex << std::setfill ('0') << std::setw(8) << regsReadback[iReg] 
                      << TLOG_ENDL;
          } // if verbose
          if (regs[iReg] != regsReadback[iReg])
          {
            spi_mismatch = true;
            size_t asicFailNum = 0;
            if (iReg > 0) asicFailNum = (iReg-1) / 9;
            TLOG_INFO(identification) << "FE-ADC ASIC " << asicFailNum << " SPI faled" << TLOG_ENDL;
          } // if regs don't match
        } // for iReg
        if (!spi_mismatch) break;
      } // for iSPIRead
      
      //TLOG_INFO(identification) << "spi_mismatch value :  " << spi_mismatch << TLOG_ENDL;
      //TLOG_INFO(identification) << "ContinueOnFEMBSPIError value :  " << ContinueOnFEMBSPIError << TLOG_ENDL;
      
      if(spi_mismatch)
      {
        if(ContinueOnFEMBSPIError)
        {
          TLOG_INFO(identification) << "FEMB ASIC SPI readback mismatch--problems communicating with ASICs for FEMB: " << int(iFEMB) << TLOG_ENDL;
        }
        else
        {
          WIBException::FEMB_SPI_READBACK_MISMATCH e;
          std::stringstream expstr;
          expstr << " for FEMB: " << int(iFEMB);
          e.Append(expstr.str().c_str());
          throw e;
        }
      } // if spi_mismatch
    } // if iSPIWrite == 1
    sleep(0.1);
    
    //TLOG_INFO(identification) << "....... WE ARE HERE(START)......." << TLOG_ENDL;
    
    //WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 9 ); // STREAM_EN and ADC_DATA_EN
    //WriteFEMB(iFEMB,0x09,(ReadFEMB(iFEMB,0x09)|0x9));
    WriteFEMB(iFEMB,0x09,9);
    if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(9, 0x09, iFEMB, 30);
    sleep(0.05);
    //WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 9 ); // STREAM_EN and ADC_DATA_EN
    //WriteFEMB(iFEMB,0x09,(ReadFEMB(iFEMB,0x09)|0x9));
    WriteFEMB(iFEMB,0x09,9);
    if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(9, 0x09, iFEMB, 30);
    
    sleep(0.1);
    
    ////// Following section is included by looking into BNL CE program (femb_config.py line 462-467)
    
    /*Write(20,3);
    Write(20,3);
    sleep(0.001);
    Write(20,0);
    Write(20,0);
    sleep(0.001);*/
    
    /////////////////////////////////////
    
    TLOG_INFO(identification) << "=== STREAM_EN : " << int(ReadFEMB(iFEMB,"STREAM_EN")) << TLOG_ENDL;
    TLOG_INFO(identification) << "=== PRBS EN : " << int(ReadFEMB(iFEMB,"PRBS_EN")) << TLOG_ENDL;
    TLOG_INFO(identification) << "=== CNT EN : " << int(ReadFEMB(iFEMB,"PRBS_EN")) << TLOG_ENDL;
    TLOG_INFO(identification) << "=== ADC DATA EN : " << int(ReadFEMB(iFEMB,"ADC_DATA_EN")) << TLOG_ENDL;
    
    //TLOG_INFO(identification) << "....... WE ARE HERE(END)......." << TLOG_ENDL;
    
    //adc_sync_status = (uint16_t) ReadFEMB(iFEMB, "ADC_ASIC_SYNC_STATUS");

    // The real sync check can happen here in Shanshan's code

  } // for iSPIWrite

  return 1;
}

uint16_t WIB::SetupASICPulserBits(uint8_t iFEMB){
  const std::string identification = "WIB::SetupASICPulserBits";
  const uint32_t REG_SPI_BASE_WRITE = 0x200; // 512
  const uint32_t  REG_SPI_BASE_READ = 0x250; // 592
  //uint16_t adc_sync_status = 0xFFFF;

  size_t nASICs = 8;
  unsigned nTries = 3;
  for(unsigned iSPIWrite=0; iSPIWrite < nTries; iSPIWrite++)
  {
    WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 0 ); // Turn off STREAM_EN and ADC_DATA_EN
    sleep(0.1);
      
    TLOG_INFO(identification) << "ASIC SPI Write Registers..." << TLOG_ENDL;

    uint8_t value = 0x2;
    uint8_t mask = 0x3;
    uint8_t pos = 24;   
    std::vector<uint32_t> vals(nASICs);
    for (size_t iASIC=0; iASIC<nASICs; iASIC++)
    {
        uint32_t address = (REG_SPI_BASE_WRITE + 9*iASIC);
        TLOG_INFO(identification) <<"Writing address " << std::hex << std::setfill ('0') << std::setw(8) << address << TLOG_ENDL;
        //WriteFEMBBits(iFEMB, address, pos, mask, value);

        //Bit-wise calculation of register val
        uint32_t shiftVal = value & mask;
        uint32_t regMask = (mask << pos);
        uint32_t initVal = ReadFEMB(iFEMB,address);
        uint32_t newVal = ( (initVal & ~(regMask)) | (shiftVal << pos) );
        vals[iASIC] = newVal;
        WriteFEMB(iFEMB,address,newVal);

        sleep(0.01);
    }
      
    //run the SPI programming
    sleep(0.1);
    WriteFEMB(iFEMB, "WRITE_ASIC_SPI", 1);
    sleep(0.1);
  
    if (iSPIWrite == nTries - 1)
    {
      // Now check readback
      bool spi_mismatch = false;
      for (unsigned iSPIRead = 0; iSPIRead < 2; iSPIRead++)
      {
        TLOG_INFO(identification) << "ASIC SPI Readback..." << TLOG_ENDL;
        std::vector<uint32_t> regsReadback(nASICs);
        for (size_t iASIC=0; iASIC<nASICs; iASIC++)
        {
            uint32_t regReadback = ReadFEMB(iFEMB, (REG_SPI_BASE_READ + 9*iASIC + 8));
            regsReadback[iASIC] = regReadback;
            sleep(0.01);
        }
  
        TLOG_INFO(identification) << "ASIC SPI register number, write val, read val:" << TLOG_ENDL;
        spi_mismatch = false;
        for (size_t iASIC=0; iASIC<nASICs; iASIC++)
        {
          TLOG_INFO(identification) << std::dec << std::setfill (' ') << std::setw(3) << iASIC 
                    << "  " 
                    << std::hex << std::setfill ('0') << std::setw(8) << vals[iASIC] 
                    << "  "
                    << std::hex << std::setfill ('0') << std::setw(8) << regsReadback[iASIC] 
                    << TLOG_ENDL;
          if (vals[iASIC] != regsReadback[iASIC])
          {
            spi_mismatch = true;
            size_t asicFailNum = 0;
            if (iASIC > 0) asicFailNum = (iASIC-1); 
            TLOG_INFO(identification) << "FE-ADC ASIC " << asicFailNum << " SPI faled" << TLOG_ENDL;
          } // if regs don't match
        } // for iReg
        if (!spi_mismatch) break;
      } // for iSPIRead
      if(spi_mismatch)
      {
         WIBException::WIB_ERROR e;
         e.Append("SPI programming failure");
         throw e;
      } // if spi_mismatch
    } // if iSPIWrite == 1
    sleep(0.1);

    WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 9 ); // STREAM_EN and ADC_DATA_EN
    sleep(0.05);
    WriteFEMB(iFEMB, "STREAM_AND_ADC_DATA_EN", 9 ); // STREAM_EN and ADC_DATA_EN
    sleep(0.1);
  
    //adc_sync_status = (uint16_t) ReadFEMB(iFEMB, "ADC_ASIC_SYNC_STATUS");

    // The real sync check can happen here in Shanshan's code

  } // for iSPIWrite

  return 1;
}

void WIB::SetupFPGAPulser(uint8_t iFEMB, uint8_t dac_val){
  const std::string identification = "WIB::SetupFPGAPulser";
  TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " Configuring FPGA pulser with DAC value: " << int(dac_val) << TLOG_ENDL;

  WriteFEMB(iFEMB, "ASIC_TP_EN", 0 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "ASIC_TP_EN", iFEMB, 30);
  WriteFEMB(iFEMB, "FPGA_TP_EN", 1 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(1, "FPGA_TP_EN", iFEMB, 30);

  WriteFEMB(iFEMB, "DAC_SELECT", 1 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(1, "DAC_SELECT", iFEMB, 30);
  //TLOG_INFO(identification) << "TEST_PULSE_AMPLITUDE (Before writting) : " << int(dac_val) << TLOG_ENDL;
  WriteFEMB(iFEMB, "TEST_PULSE_AMPLITUDE", dac_val );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(dac_val, "TEST_PULSE_AMPLITUDE", iFEMB, 30);
  //TLOG_INFO(identification) << "TEST_PULSE_AMPLITUDE (After writting) : " << int(dac_val) << TLOG_ENDL;
  WriteFEMB(iFEMB, "TEST_PULSE_DELAY", 219 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(219, "TEST_PULSE_DELAY", iFEMB, 30);
  WriteFEMB(iFEMB, "TEST_PULSE_PERIOD", 497 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(497, "TEST_PULSE_PERIOD", iFEMB, 30);

  WriteFEMB(iFEMB, "INT_TP_EN", 0 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "INT_TP_EN", iFEMB, 30);
  //WriteFEMB(iFEMB, "EXT_TP_EN", 1 );
  WriteFEMB(iFEMB, "EXP_TP_EN", 1 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(1, "EXP_TP_EN", iFEMB, 30);
}

void WIB::SetupInternalPulser(uint8_t iFEMB){
  const std::string identification = "WIB::SetupInternalPulser";
  TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " Configuring internal pulser" << TLOG_ENDL;

  WriteFEMB(iFEMB, "DAC_SELECT", 0 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "DAC_SELECT", iFEMB, 30);
  WriteFEMB(iFEMB, "TEST_PULSE_AMPLITUDE", 0 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "TEST_PULSE_AMPLITUDE", iFEMB, 30);
  WriteFEMB(iFEMB, "TEST_PULSE_DELAY", 219 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(219, "TEST_PULSE_DELAY", iFEMB, 30);
  WriteFEMB(iFEMB, "TEST_PULSE_PERIOD", 497 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(497, "TEST_PULSE_PERIOD", iFEMB, 30);

  WriteFEMB(iFEMB, "INT_TP_EN", 0 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "INT_TP_EN", iFEMB, 30);
  //WriteFEMB(iFEMB, "EXT_TP_EN", 1 );
  WriteFEMB(iFEMB, "EXP_TP_EN", 1 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(1, "EXP_TP_EN", iFEMB, 30);

  WriteFEMB(iFEMB, "FPGA_TP_EN", 0 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, "FPGA_TP_EN", iFEMB, 30);
  WriteFEMB(iFEMB, "ASIC_TP_EN", 1 );
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(1, "ASIC_TP_EN", iFEMB, 30);
}

/////////////////////////////////////////////

/*void WIB::SetupInternalPulser(uint8_t iFEMB, uint8_t dac_val){
  const std::string identification = "WIB::SetupInternalPulser";
  TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " Configuring internal pulser with DAC value : " << int(dac_val) << TLOG_ENDL;

  WriteFEMB(iFEMB, "DAC_SELECT", 0 );
  WriteFEMB(iFEMB, "TEST_PULSE_AMPLITUDE", dac_val );
  WriteFEMB(iFEMB, "TEST_PULSE_DELAY", 219 );
  WriteFEMB(iFEMB, "TEST_PULSE_PERIOD", 497 );

  WriteFEMB(iFEMB, "INT_TP_EN", 0 );
  //WriteFEMB(iFEMB, "EXT_TP_EN", 1 );
  WriteFEMB(iFEMB, "EXP_TP_EN", 1 );

  WriteFEMB(iFEMB, "FPGA_TP_EN", 0 );
  WriteFEMB(iFEMB, "ASIC_TP_EN", 1 );
}*/


////////////////////////////////////////////

void WIB::WriteFEMBPhase(uint8_t iFEMB, uint16_t clk_phase_data){
  const std::string identification = "WIB::WriteFEMBPhase";
  uint32_t clk_phase_data0 = (clk_phase_data >> 8) & 0xFF;
  uint32_t clk_phase_data1 = clk_phase_data & 0xFF;
 
  uint32_t data;
  uint32_t read_back;
  int count = 0;
  sleep(0.001); 
  for(int i = 0; i < 10; ++i){

    data = (~(clk_phase_data0)) & 0xFF;
    WriteFEMB(iFEMB, 6, data);
    sleep(0.001); 
    read_back = ReadFEMB(iFEMB, 6);
    sleep(0.001); 
    if ( (read_back & 0xFF) == ((~(clk_phase_data0)) & 0xFF) ){
      break;
    }
    else{
      count++;
    }
  }
  if( count >= 10){
    TLOG_INFO(identification) << "readback val for 0 is different from written data "<< TLOG_ENDL;
    //Put in system exit?
  }
  count = 0;
  for(int i = 0; i < 10; ++i){
    
    data = (~(clk_phase_data1)) & 0xFF; 
    WriteFEMB(iFEMB, 15, data);
    sleep(0.001); 
    read_back = ReadFEMB(iFEMB, 15);
    sleep(0.001); 
    if ( (read_back & 0xFF) == ((~(clk_phase_data1)) & 0xFF) ){
      break;
    }  
    else{
      count++;
    }
  }
  if( count >= 10){
    TLOG_INFO(identification) << "readback val for 1 is different from written data "<< TLOG_ENDL;
    //Put in system exit?
  }
  count = 0;
  for(int i = 0; i < 10; ++i){
    
    data = clk_phase_data0;
    WriteFEMB(iFEMB, 6, data);
    sleep(0.001);
    
    read_back = ReadFEMB(iFEMB,6);
    sleep(0.001); 
    if( (read_back & 0xFF) == ((clk_phase_data0) & 0xFF) ){
      break; 
    }
    else{
      count++;
    }
  }
  if( count >= 10){
    TLOG_INFO(identification) << "readback val for 2 is different from written data "<< TLOG_ENDL;
    //Put in system exit?
  }
  count = 0;
  for(int i = 0; i < 10; ++i){
    
    data = clk_phase_data1;
    WriteFEMB(iFEMB, 15, data);
    sleep(0.001);
    
    read_back = ReadFEMB(iFEMB,6);
    sleep(0.001); 
    if( (read_back & 0xFF) == ((clk_phase_data1) & 0xFF) ){
      break; 
    }
    else{
      count++;
    }
  }
  if( count >= 10){
    TLOG_INFO(identification) << "readback val for 3 is different from written data "<< TLOG_ENDL;
    //Put in system exit?
  }
}

bool WIB::TryFEMBPhases(uint8_t iFEMB, std::vector<uint16_t> phases){
  const std::string identification = "WIB::TryFEMBPhases";
  size_t nPhases = phases.size();
  TLOG_INFO(identification) << "Searching " << nPhases << " sets of phases:" << TLOG_ENDL;
  for(size_t ip = 0; ip < nPhases; ++ip){
    uint16_t phase = phases.at(ip);
    TLOG_INFO(identification) << "Set " << ip << TLOG_ENDL;
    TLOG_INFO(identification) << "\t Phase: " << std::hex << std::setw(4) << phase << TLOG_ENDL;

    WriteFEMBPhase(iFEMB,phase);
    //If it made it this far, it found the correct readback val
    sleep(0.001);
    uint32_t adc_fifo_sync = (ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16;
    sleep(0.001);
    adc_fifo_sync = (ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16; 
    sleep(0.001);

    TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " ADC FIFO sync: " << std::bitset<16>(adc_fifo_sync) << TLOG_ENDL;
    
    if (adc_fifo_sync == 0){
      TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " ADC FIFO synced" << TLOG_ENDL;
      TLOG_INFO(identification) << "  phase:   " << std::hex << std::setw(4) << std::setfill('0') << (uint32_t) phase << TLOG_ENDL;
      return true;
    }
  } 

  //std::cout << "Could not find successful phase" << std::endl;
  return false;
}

bool WIB::HuntFEMBPhase(uint8_t iFEMB, uint16_t clk_phase_data_start){
  const std::string identification = "WIB::HuntFEMBPhase";
  uint32_t clk_phase_data0 = (clk_phase_data_start >> 8) & 0xFF;
  uint32_t clk_phase_data1 = clk_phase_data_start & 0xFF;
  uint32_t adc_fifo_sync = 1;

  uint32_t a_cs[8] = {
    0xc000, 0x3000, 0x0c00, 0x0300,
    0x00c0, 0x0030, 0x000c, 0x0003};

  uint32_t a_mark[8] = {
    0x80, 0x40, 0x20, 0x10,
    0x08, 0x04, 0x02, 0x01};

  uint32_t a_cnt[8] = {
    0,0,0,0,
    0,0,0,0};

  while (adc_fifo_sync){
    sleep(0.001);
    adc_fifo_sync = (ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16;
    sleep(0.001);
    adc_fifo_sync = (ReadFEMB(iFEMB, 6) & 0xFFFF0000) >> 16; 
    sleep(0.001);

    TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " ADC FIFO sync: " << std::bitset<16>(adc_fifo_sync) << TLOG_ENDL;
    
    if (adc_fifo_sync == 0){
      TLOG_INFO(identification) << "FEMB " << int(iFEMB) << " Successful SPI config and ADC FIFO synced" << TLOG_ENDL;
      //std::cout << "  ADC_ASIC_CLK_PHASE_SELECT:   " << std::hex << std::setw(2) << std::setfill('0') << clk_phase_data0 << std::endl;
      //std::cout << "  ADC_ASIC_CLK_PHASE_SELECT_2: " << std::hex << std::setw(2) << std::setfill('0') << clk_phase_data1 << std::endl;
      TLOG_INFO(identification) << "  phase:   " << std::hex << std::setw(2) << std::setfill('0') << clk_phase_data0;
      TLOG_INFO(identification) << std::hex << std::setw(2) << std::setfill('0') << clk_phase_data1 << TLOG_ENDL;
      return true;
    }
    else{
      TLOG_INFO(identification) << "ERROR: sync not zero: " << std::bitset<16>(adc_fifo_sync) << TLOG_ENDL;
      for(int i = 0; i < 8; ++i){
        uint32_t a = adc_fifo_sync & a_cs[i];
        uint32_t a_mark_xor = 0;
        if (a != 0){
          a_cnt[i]++;
          a_mark_xor = a_mark[i] ^ 0xFF;
          if (a_cnt[i] == 1 || a_cnt[i] == 3){
            clk_phase_data0 = ((clk_phase_data0 & a_mark[i]) ^ a_mark[i]) + (clk_phase_data0 & a_mark_xor);
          }
          else if (a_cnt[i] == 2 || a_cnt[i] == 4){
            clk_phase_data1 = ((clk_phase_data1 & a_mark[i]) ^ a_mark[i]) + (clk_phase_data1 & a_mark_xor);
          }
          else if (a_cnt[i] >= 5){
            return false;
          }
        }
        else{
          continue;
        }
      }
      uint16_t clk_phase_to_write=0;
      clk_phase_to_write |= (clk_phase_data0 & 0xFF) << 8;
      clk_phase_to_write |= clk_phase_data1;
      WriteFEMBPhase(iFEMB,clk_phase_to_write);
    }
  }
  return false;
}

void WIB::ConfigFEMBMode(uint8_t iFEMB, uint32_t pls_cs, uint32_t dac_sel, uint32_t fpga_dac, uint32_t asic_dac, uint32_t mon_cs = 0){
  const std::string identification = "WIB::ConfigFEMBMode";
  uint32_t tp_sel;
  if (mon_cs == 0){
    tp_sel = ((asic_dac & 0x01) << 1) + (fpga_dac & 0x01) + ((dac_sel&0x1)<<8);
  }
  else{
    tp_sel = 0x402;
  }
  WriteFEMB(iFEMB, 16, tp_sel & 0x0000FFFF);
  WriteFEMB(iFEMB, 18, 0x11);
  uint32_t pls_cs_value = 0x00;
  if (pls_cs == 0) pls_cs_value = 0x11;//disable all
  else if (pls_cs == 1) pls_cs_value = 0x10;//internal pls
  else if (pls_cs == 2) pls_cs_value = 0x01;//external pls

  WriteFEMB(iFEMB, 18, pls_cs_value);
}

void WIB::SetContinueOnFEMBRegReadError(bool enable){
  const std::string identification = "WIB::SetContinueOnFEMBRegReadError";
  ContinueOnFEMBRegReadError = enable;
}

void WIB::SetContinueOnFEMBSPIError(bool enable){
  const std::string identification = "WIB::SetContinueOnFEMBSPIError";
  ContinueOnFEMBSPIError = enable;
}

void WIB::SetContinueOnFEMBSyncError(bool enable){
  const std::string identification = "WIB::SetContinueOnFEMBSyncError";
  ContinueOnFEMBSyncError = enable;
}

void WIB::SetContinueIfListOfFEMBClockPhasesDontSync(bool enable){
  const std::string identification = "WIB::SetContinueIfListOfFEMBClockPhasesDontSync";
  ContinueIfListOfFEMBClockPhasesDontSync = enable;
}

void WIB::CheckFEMBRegisters(uint32_t expected_val, std::string reg_addrs, int FEMB_NO, int tries){
  const std::string identification = "WIB::CheckFEMBRegisters";
  bool throw_excpt = true;
  uint32_t reg_value = 9999; // earlier -9999
  for (int i = 0; i < tries; i++){
       sleep(0.1);
       reg_value = ReadFEMB(FEMB_NO, reg_addrs);
       if (reg_value == expected_val){
           throw_excpt = false;
	   break;
       }
       WriteFEMB(FEMB_NO, reg_addrs, expected_val);
  }
  if(throw_excpt){
     WIBException::FEMB_REG_READ_ERROR e;
     std::stringstream expstr;
     expstr << "FEMB " << FEMB_NO << " register " << reg_addrs << " value is " << reg_value << " and expected value is " << expected_val;
     e.Append(expstr.str().c_str());
     throw e;
  }
}

void WIB::CheckFEMBRegisters(uint32_t expected_val, uint32_t reg_addrs, int FEMB_NO, int tries){
  const std::string identification = "WIB::CheckFEMBRegisters";
  bool throw_excpt = true;
  uint32_t reg_value = 9999; // earlier -9999
  for (int i = 0; i < tries; i++){
       sleep(0.1);
       reg_value = ReadFEMB(FEMB_NO, reg_addrs);
       if (reg_value == expected_val){
           throw_excpt = false;
	   break;
       }
       WriteFEMB(FEMB_NO, reg_addrs, expected_val);
  }
  if(throw_excpt){
     WIBException::FEMB_REG_READ_ERROR e;
     std::stringstream expstr;
     expstr << "FEMB " << FEMB_NO << " register " << reg_addrs << " value is " << reg_value << " and expected value is " << expected_val;
     e.Append(expstr.str().c_str());
     throw e;
  }
}

void WIB::New_ConfigFEMB(uint8_t iFEMB,uint8_t config_no,std::vector<uint32_t> fe_config,std::vector<uint16_t> clk_phases,uint8_t pls_mode, uint8_t pls_dac_val,uint8_t start_frame_mode_sel, uint8_t start_frame_swap){
  const std::string identification = "WIB::New_ConfigFEMB";
  // This function is written as a temporary solution to some of the problems, we
  // see in running the WIB board reader (Sometimes all FEMBs are not configured at once).
  
  // This function will be replacning the existing ConfigFEMB function in wibtools.
  
  // This function is a modified version of existing ConfigFEMB function in wibtools with
  // some of the register writes removed and added compared to that of existing SetupFEMBASICs. (all modification are minor)
  
  // All modifications were made by looking at "CE_CHK_CFG" function in the "cls_config.py" module 
  // inside the repository "CE_LD"(git branch name is, Installation_Support)
  
  TLOG_INFO(identification) << "************* Now Starting New_ConfigFEMB  ****************" << TLOG_ENDL;
  
  if (iFEMB < 1 || iFEMB > 4)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMB: iFEMB should be between 1 and 4: "
            << int(iFEMB);
     e.Append(expstr.str().c_str());
     throw e;
  }
  if (pls_mode > 2)
  {
    WIBException::WIB_BAD_ARGS e;
    std::stringstream expstr;
    expstr << "ConfigFEMB: pls_dac_mode is allowed to be 0 (off), 1 (FPGA), 2 (internal), but is: "
           << int(pls_mode);
    e.Append(expstr.str().c_str());
    throw e;
  }
  if (start_frame_mode_sel > 1 || start_frame_swap > 1)
  {
     WIBException::WIB_BAD_ARGS e;
     std::stringstream expstr;
     expstr << "ConfigFEMB: start_frame_mode_sel and start_frame_swap must be 0 or 1";
     e.Append(expstr.str().c_str());
     throw e;
  }
  
  if(fe_config.size() != 8){

    WIBException::WIB_BAD_ARGS e;
    std::stringstream expstr;
    expstr << "Error: Expecting 9 Front End configuration options:" << std::endl <<
    "\t0: Gain" << std::endl << 
    "\t1: Shaping Time" << std::endl << 
    "\t2: High Baseline" << std::endl << 
    "\t3: High Leakage" << std::endl << 
    "\t4: Leakage x 10" << std::endl << 
    "\t5: AC Coupling" << std::endl << 
    "\t6: Buffer" << std::endl << 
    "\t7: Use External Clock" << std::endl;     
    e.Append(expstr.str().c_str());
    throw e;
  }
  else{
    TLOG_INFO(identification) << "Front End configuration options:" << TLOG_ENDL <<  
    "\t0:" << std::setw(22) << std::setfill(' ') <<  "Gain "               << fe_config[0] << TLOG_ENDL <<
    "\t1:" << std::setw(22) << std::setfill(' ') <<  "Shaping Time "       << fe_config[1] << TLOG_ENDL <<
    "\t2:" << std::setw(22) << std::setfill(' ') <<  "High Baseline "      << fe_config[2] << TLOG_ENDL <<
    "\t3:" << std::setw(22) << std::setfill(' ') <<  "High Leakage "       << fe_config[3] << TLOG_ENDL <<
    "\t4:" << std::setw(22) << std::setfill(' ') <<  "Leakage x 10 "       << fe_config[4] << TLOG_ENDL <<
    "\t5:" << std::setw(22) << std::setfill(' ') <<  "AC Coupling "        << fe_config[5] << TLOG_ENDL <<
    "\t6:" << std::setw(22) << std::setfill(' ') <<  "Buffer "             << fe_config[6] << TLOG_ENDL <<
    "\t7:" << std::setw(22) << std::setfill(' ') <<  "Use External Clock " << fe_config[7] << TLOG_ENDL; 
  }
  
  if(ReadFEMB(iFEMB,"VERSION_ID") == ReadFEMB(iFEMB,"SYS_RESET")) { // can't read register if equal
    if(ContinueOnFEMBRegReadError){
      TLOG_INFO(identification) << "Error: Can't read registers from FEMB " << int(iFEMB) << TLOG_ENDL;
      return;
    }
    WIBException::FEMB_REG_READ_ERROR e;
    std::stringstream expstr;
    expstr << " for FEMB: " << int(iFEMB);
    e.Append(expstr.str().c_str());
    throw e;
  }
  
  WriteFEMB(iFEMB, 0, 1);
  sleep(0.001);
  
  // Setup pulser
  uint8_t internal_daq_value = 0;
  if (pls_mode == 1) // internal, FE ASIC, 6 bits
  {
    TLOG_INFO(identification) << "************ USING INTERNAL PULSAR FOR CALIBRATION *****************" << TLOG_ENDL;
    
    if (pls_dac_val > 63)
    {
      WIBException::WIB_BAD_ARGS e;
      std::stringstream expstr;
      expstr << "ConfigFEMB: pls_dac_val is 6 bits for internal DAC, must be 0-63, but is: "
             << int(pls_dac_val);
      e.Append(expstr.str().c_str());
      throw e;
    }
    internal_daq_value = pls_dac_val;
    SetupInternalPulser(iFEMB);
  }
  else if (pls_mode == 2) // external, FPGA, 6 bits
  {
    TLOG_INFO(identification) << "************ USING EXTERNAL PULSAR FOR CALIBRATION *****************" << TLOG_ENDL;
    
    if (pls_dac_val > 63)
    {
      WIBException::WIB_BAD_ARGS e;
      std::stringstream expstr;
      expstr << "ConfigFEMB: pls_dac_val is 6 bits for FPGA DAC, must be 0-63, but is: "
             << int(pls_dac_val);
      e.Append(expstr.str().c_str());
      throw e;
    }
    
    SetupFPGAPulser(iFEMB,pls_dac_val);
  }
  
  WriteFEMB(iFEMB, 42, 0);
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(0, 42, iFEMB, 30);
  sleep(0.001);
  
  New_SetupFEMBASICs(iFEMB, config_no, fe_config[0], fe_config[1], fe_config[2], fe_config[3], fe_config[4], fe_config[5], fe_config[6], fe_config[7], pls_mode, internal_daq_value);
  
  WriteFEMB(iFEMB,9, 9);
  if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(9, 9, iFEMB, 30);
  
  Write(1, 0);
  Write(1, 0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, 1, 30);
  Write(1, 2);
  Write(1, 2);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(2, 1, 30);
  Write(1, 0);
  Write(1, 0);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0, 1, 30);
  //
  ////Reset Error /WIB
  Write(18, 0x8000);
  Write(18, 0x8000);
  if (CheckWIB_FEMB_REGs) CheckWIBRegisters(0x8000, 18, 30);
  
  FEMB_UDPACQ(iFEMB-1);
  
  TLOG_INFO(identification) << "************* New_ConfigFEMB completed ****************" << TLOG_ENDL;
}

uint16_t WIB::New_SetupFEMBASICs(uint8_t iFEMB, uint8_t config_no, uint8_t gain, uint8_t shape, uint8_t highBaseline, bool highLeakage, bool leakagex10, bool acCoupling,bool buffer, bool useExtClock, uint8_t internalDACControl, uint8_t internalDACValue)
{
  // This function is written as a temporary solution to some of the problems, we
  // see in running the WIB board reader (Sometimes all FEMBs are not configured at once).
  
  // This function will be replacning the existing SetupFEMBASICs function in wibtools.
  
  // This function is a modified version of existing SetupFEMBASICs function in wibtools with
  // some of the register writes removed from the existing SetupFEMBASICs. (very tiny modifications)
  
  // All modifications were made by looking at "CE_CHK_CFG" function in the "cls_config.py" module 
  // inside the repository "CE_LD"(git branch name is, Installation_Support)
  
  const std::string identification = "WIB::New_SetupFEMBASICs";
  
  TLOG_INFO(identification) << "************* Now Starting New_SetupFEMBASICs  ****************" << TLOG_ENDL;
  
  (void) buffer; // to make compiler not complain about unused arguments

  if (gain > 3) 
  {
       WIBException::WIB_BAD_ARGS e;
       std::stringstream expstr;
       expstr << "gain should be between 0 and 3, but is: "
              << int(gain);
       e.Append(expstr.str().c_str());
       throw e;
  }
  if (shape > 3) 
  {
       WIBException::WIB_BAD_ARGS e;
       std::stringstream expstr;
       expstr << "shape should be between 0 and 3, but is: "
              << int(shape);
       e.Append(expstr.str().c_str());
       throw e;
  }

  const size_t REG_SPI_BASE_WRITE = 0x200; // 512
  const size_t REG_SPI_BASE_READ = 0x250; // 592
  // for COTS ADCs first 36 registers are used for the FE ASICs

  bool bypassOutputBuffer=true; // if false might blow up SBND
  bool useOutputMonitor=false; // if true might blow up SBND
  bool useCh16HighPassFilter=false;
  bool monitorBandgapNotTemp=false;
  bool monitorTempBandgapNotSignal=false;
  bool useTestCapacitance = (bool) internalDACControl;
  
  // Flip bits of gain
  if (gain == 0x1) gain = 0x2;
  else if (gain== 0x2) gain = 0x1;

  // Shape
  if (shape == 0x0) shape = 0x2; // 0.5 us
  else if (shape == 0x1) shape = 0x0; // 1 us
  else if (shape == 0x2) shape = 0x3; // 2 us
  else if (shape == 0x3) shape = 0x1; // 3 us
  
  FE_ASIC_reg_mapping fe_map;
  if (highBaseline > 1)
  {
    // Set them all to high baseline
    fe_map.set_board(useTestCapacitance,0,gain,shape,
                      useOutputMonitor,1,!highLeakage,
                      monitorBandgapNotTemp,monitorTempBandgapNotSignal,useCh16HighPassFilter,
                      leakagex10,acCoupling,internalDACControl,internalDACValue
                  );
    // Now just set collection channels to low baseline
    //fe_map.set_collection_baseline(1); // !bypassOutputBuffer
    fe_map.set_collection_baseline(1,config_no);
  }
  else
  {
    fe_map.set_board(useTestCapacitance,!highBaseline,gain,shape,
                      useOutputMonitor,1,!highLeakage,
                      monitorBandgapNotTemp,monitorTempBandgapNotSignal,useCh16HighPassFilter,
                      leakagex10,acCoupling,internalDACControl,internalDACValue
                  );
  }
  
  ASIC_reg_mapping map;
  map.set_board(fe_map);
  map.get_regs();
  const std::vector<uint32_t> regs = map.get_regs();
  const size_t nRegs = regs.size();
  
  for(unsigned iSPIWrite=0; iSPIWrite < 2; iSPIWrite++)
  {
    for (size_t iReg=0; iReg<nRegs; iReg++)
    {
        WriteFEMB(iFEMB,REG_SPI_BASE_WRITE+iReg,regs[iReg]);
	sleep(0.01);
	if (CheckWIB_FEMB_REGs) CheckFEMBRegisters(regs[iReg], REG_SPI_BASE_WRITE+iReg, iFEMB, 30);
    }
    
    //run the SPI programming
    sleep(0.1);
    
    WriteFEMB(iFEMB,0x02,1);
    sleep(0.1);
    
    WriteFEMB(iFEMB,0x02,1);
    sleep(0.1);
    
    WriteFEMB(iFEMB,0x02,1);
    sleep(0.1);
  
    if (iSPIWrite == 1)
    {
      // Now check readback
      bool spi_mismatch = false;
      for (unsigned iSPIRead = 0; iSPIRead < 2; iSPIRead++)
      {
        TLOG_INFO(identification)<< "ASIC SPI Readback..." << TLOG_ENDL;
        std::vector<uint32_t> regsReadback(nRegs);
        for (size_t iReg=0; iReg<nRegs; iReg++)
        {
            uint32_t regReadback = ReadFEMB(iFEMB,REG_SPI_BASE_READ+iReg);
            regsReadback[iReg] = regReadback;
            sleep(0.01);
        }
  
        bool verbose = false;
        if (verbose) TLOG_INFO(identification) << "ASIC SPI register number, write val, read val:" << TLOG_ENDL;
        spi_mismatch = false;
        for (size_t iReg=0; iReg<nRegs; iReg++)
        {
          if (verbose)
          {
            TLOG_INFO(identification) << std::dec << std::setfill (' ') << std::setw(3) << iReg 
                      << "  " 
                      << std::hex << std::setfill ('0') << std::setw(8) << regs[iReg] 
                      << "  "
                      << std::hex << std::setfill ('0') << std::setw(8) << regsReadback[iReg] 
                      << TLOG_ENDL;
          } // if verbose
          if (regs[iReg] != regsReadback[iReg])
          {
            spi_mismatch = true;
            size_t asicFailNum = 0;
            if (iReg > 0) asicFailNum = (iReg-1) / 9;
            TLOG_INFO(identification) << "FE-ADC ASIC " << asicFailNum << " SPI faled" << TLOG_ENDL;
          } // if regs don't match
        } // for iReg
        if (!spi_mismatch) break;
      } // for iSPIRead
      
      if(spi_mismatch)
      {
        if(ContinueOnFEMBSPIError)
        {
          TLOG_INFO(identification) << "FEMB ASIC SPI readback mismatch--problems communicating with ASICs for FEMB: " << int(iFEMB) << TLOG_ENDL;
        }
        else
        {
          WIBException::FEMB_SPI_READBACK_MISMATCH e;
          std::stringstream expstr;
          expstr << " for FEMB: " << int(iFEMB);
          e.Append(expstr.str().c_str());
          throw e;
        }
      } // if spi_mismatch
    } // if iSPIWrite == 1
  } // for iSPIWrite
  
  TLOG_INFO(identification) << "************* New_SetupFEMBASICs completed ****************" << TLOG_ENDL;
  return 1;
}

void WIB::CE_CHK_CFG(uint32_t iFEMB, uint8_t config_no, bool test_chnl_map, uint32_t chnl_no, uint32_t pls_cs, uint32_t dac_sel, uint32_t fpgadac_en,                         uint32_t asicdac_en,uint32_t fpgadac_v, uint32_t pls_gap, uint32_t pls_dly, uint32_t mon_cs, uint32_t data_cs, uint32_t sts,                             uint32_t snc, uint32_t sg0, uint32_t sg1, uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf,
		     uint32_t slk0, uint32_t stb1, uint32_t stb, uint32_t s16, uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2,                                 uint32_t dac,bool fecfg_loadflg){
   // This function is copied from Shanshan's python script
   // to configure WIB/FEMB.
   // The original function is in cls_config.py module inside the repository CE_LD with same name (git branch name is, Installation_Support)
   
   const std::string identification = "WIB::CE_CHK_CFG";
   
   TLOG_INFO(identification) << "************* Now Starting CE_CHK_CFG  ****************" << TLOG_ENDL;
   
   uint32_t tp_sel;
   
   if (mon_cs == 0) tp_sel = ((asicdac_en&0x01) <<1) + (fpgadac_en&0x01) + ((dac_sel&0x1)<<8);
   else tp_sel = 0x402;
   
   TLOG_INFO(identification) << "tp_sel value : " << tp_sel << TLOG_ENDL;
   
   uint32_t pls_cs_value = 0x3;
   
   if (pls_cs == 0) pls_cs_value = 0x3; //disable all
   else if (pls_cs == 1) pls_cs_value = 0x2; // internal pls
   else if (pls_cs == 2) pls_cs_value = 0x1; // external pls
   else if (pls_cs == 3) pls_cs_value = 0x0; // enable int and ext pls
   
   TLOG_INFO(identification) << "pls_cs value : " << pls_cs_value << TLOG_ENDL;
   
   uint32_t reg_5_value;
   
   if (fpgadac_en == 1){
      reg_5_value = ((pls_gap<<16)&0xFFFF0000) + ((pls_dly<<8)&0xFF00) + (fpgadac_v&0xFF);
   }
   
   else{
      reg_5_value = ((pls_gap<<16)&0xFFFF0000) + ((pls_dly<<8)&0xFF00) + (0x00);
   }
   
   TLOG_INFO(identification) << "Register 5 value : " << reg_5_value << TLOG_ENDL;
   
   WriteFEMB(iFEMB,0,1);
   sleep(0.001);
   WriteFEMB(iFEMB,5,reg_5_value);
   CheckFEMBRegisters(reg_5_value,5,iFEMB,30);
   WriteFEMB(iFEMB,16,tp_sel&0x0000ffff);
   CheckFEMBRegisters(tp_sel&0x0000ffff,16,iFEMB,30);
   WriteFEMB(iFEMB,18,pls_cs_value);
   CheckFEMBRegisters(pls_cs_value,18,iFEMB,30);
   
   if ((data_cs&0x0F) != 0){
      WriteFEMB(iFEMB,42,((((iFEMB-1)&0x0F)<<4) + (data_cs&0x0F)));
      CheckFEMBRegisters(((((iFEMB-1)&0x0F)<<4) + (data_cs&0x0F)),42,iFEMB,30);
   } 
   
   else{
     WriteFEMB(iFEMB,42,0);
     CheckFEMBRegisters(0,42,iFEMB,30);
   }
   
   BNL_FE_Reg_Mapping FEREG_MAP;
   std::vector<bool> regs;
   
   if (fecfg_loadflg) regs = {};
   else{
     if (test_chnl_map) FEREG_MAP.set_fe_board_for_chnl_testing(config_no,chnl_no, sts, snc, sg0, sg1, st0, st1, smn, sdf, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
     else{
       FEREG_MAP.set_fe_board(config_no,sts, snc, sg0, sg1, st0, st1, smn, sdf, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac); // ch. by ch.
       
       //FEREG_MAP.set_fe_board(sts, snc, sg0, sg1, st0, st1, smn, sdf, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac); // global configuration
       //FEREG_MAP.set_collection_fe_board(config_no, sts, snc, sg0, sg1, st0, st1, smn, sdf, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac); // set only collection plane channels
     }
     regs = FEREG_MAP.REGS;
   }
   
   std::vector<uint32_t>fe_regs((8+1)*4,0x00000000);
   std::vector<int> range_vec = {0,2,4,6};
   
   //std::ofstream debug_file,debug_file_1,debug_file_2,debug_file_3,debug_file_4;
   
   //debug_file.open("/home/nfs/sbnd/DAQ_DevAreas/DAQ_30Mar2023_VM/localProducts_sbndaq_v1_08_01_e20_prof_s112/wibtools/v1_08_00/config/debug_file_1.txt");
//debug_file_1.open("/home/nfs/sbnd/DAQ_DevAreas/DAQ_30Mar2023_VM/localProducts_sbndaq_v1_08_01_e20_prof_s112/wibtools/v1_08_00/config/debug_fe_reg_values.txt");
//debug_file_2.open("/home/nfs/sbnd/DAQ_DevAreas/DAQ_30Mar2023_VM/localProducts_sbndaq_v1_08_01_e20_prof_s112/wibtools/v1_08_00/config/debug_fe_reg_writes.txt");
//debug_file_3.open("/home/nfs/sbnd/DAQ_DevAreas/DAQ_30Mar2023_VM/localProducts_sbndaq_v1_08_01_e20_prof_s112/wibtools/v1_08_00/config/debug_fe_reg_reads.txt");
//debug_file_4.open("/home/nfs/sbnd/DAQ_DevAreas/DAQ_30Mar2023_VM/localProducts_sbndaq_v1_08_01_e20_prof_s112/wibtools/v1_08_00/config/debug_fe_all_regs.txt");

   /*for (unsigned int i=0; i<regs.size(); i++) debug_file_4 << i << " Value : " << regs[i] << "\n";
   debug_file_4.close();*/
   
   for (unsigned int i=0; i<range_vec.size(); i++){
       //debug_file << "************** Range vector value : " << range_vec[i] << " *****************\n";
       int chip_bits_len = 8*(16+2);
       
       std::vector<bool> chip_fe_regs0(((range_vec[i]+1)* chip_bits_len)-(range_vec[i]*chip_bits_len));
       copy(regs.begin()+(range_vec[i]*chip_bits_len), regs.begin()+((range_vec[i]+1)* chip_bits_len), chip_fe_regs0.begin());
       
       std::vector<bool> chip_fe_regs1(((range_vec[i]+2)* chip_bits_len)-((range_vec[i]+1)*chip_bits_len));
       copy(regs.begin()+((range_vec[i]+1)*chip_bits_len), regs.begin()+((range_vec[i]+2)* chip_bits_len), chip_fe_regs1.begin());
       
       //TLOG_INFO(identification) << "Length of chip_fe_regs0 vector : " << chip_fe_regs0.size() << TLOG_ENDL;
       //TLOG_INFO(identification) << "Length of chip_fe_regs1 vector : " << chip_fe_regs1.size() << TLOG_ENDL;
       //TLOG_INFO(identification) << "Range vector value : " << range_vec[i] << TLOG_ENDL;
       //debug_file << "====== Size of the chip_fe_regs0 vector : " << chip_fe_regs0.size() 
                  //<< " Size of the chip_fe_regs1 vector : " <<   chip_fe_regs1.size() << " ============\n";
       /*for (unsigned int ele=0; ele<chip_fe_regs0.size(); ele++){
          TLOG_INFO(identification) << ele << " chip reg 0 : " << chip_fe_regs0[ele] << "  chip reg 1 : " << chip_fe_regs1[ele] << TLOG_ENDL;
	  debug_file << ele << " chip reg 0 : " << chip_fe_regs0[ele] << " chip reg 1 : " << chip_fe_regs1[ele] << "\n";
       }*/
       
       std::vector<bool> chip_regs;
       
       for (unsigned int onebit=0; onebit<chip_fe_regs0.size(); onebit++){
          chip_regs.push_back(chip_fe_regs0[onebit]); 
       }
       
       for (unsigned int onebit=0; onebit<chip_fe_regs1.size(); onebit++){
          chip_regs.push_back(chip_fe_regs1[onebit]); 
       }
       
       int len32 = floor(chip_regs.size()/32);
       
       if (len32 != 9){
          WIBException::WIB_ERROR e;
          std::stringstream expstr;
          expstr << " for FEMB: " << int(iFEMB) << " ERROR FE register mapping ";
          e.Append(expstr.str().c_str());
          throw e;  
       }
       
       else{
         for (int j=0; j<len32; j++){
	    if(j*32 <= int(chip_regs.size())){
	       std::vector<bool> bits32(((j+1)*32)-(j*32));
	       copy(chip_regs.begin()+(j*32), chip_regs.begin()+((j+1)*32), bits32.begin());
	       int sum=0;
	       for (unsigned int k=0; k<bits32.size(); k++){
	          sum = sum + (bits32[k]<<k); 
	       }
	       fe_regs[int(range_vec[i]/2*len32) + j] = sum;
	    }
	 }
       } 
   } // loop over range vector
   
   //debug_file.close();
   
   int i = 0;
   
   /*for (unsigned int i=0; i<fe_regs.size(); i++){
      debug_file_1 << i << "  " << fe_regs[i] << "\n";
   }*/
   
   //debug_file_1.close();
   
   for (int regNum=0x200; regNum<(0x200+int(fe_regs.size())); regNum++){
      WriteFEMB(iFEMB,regNum,fe_regs[i]);
      CheckFEMBRegisters(fe_regs[i],regNum,iFEMB,30);
      //debug_file_2 << "Register : " << regNum << " Value : " << fe_regs[i] << "\n";
      i++; 
   }
   
   //debug_file_2.close();
   
   WriteFEMB(iFEMB,2,1); // SPI write
   //CheckFEMBRegisters(1,2,iFEMB,30);
   sleep(1);
   WriteFEMB(iFEMB,2,1); // SPI write
   //CheckFEMBRegisters(1,2,iFEMB,30);
   sleep(1);
   WriteFEMB(iFEMB,2,1); // SPI write
   //CheckFEMBRegisters(1,2,iFEMB,30);
   sleep(1);
   
   std::vector<uint32_t> fe_rb_regs;
   
   for (int regNum=0x250; regNum<(0x250+int(fe_regs.size())); regNum++){
      auto val = ReadFEMB(iFEMB,regNum);
      //sleep(0.1);
      //val = ReadFEMB(iFEMB,regNum);
      //sleep(0.1);
      //val = ReadFEMB(iFEMB,regNum);
      //debug_file_3 << "Register : " << regNum << " Value 1 : " << val << " Value 2 : " << ReadFEMB(iFEMB,regNum) << "\n";
      fe_rb_regs.push_back(val);
   }
   
   //debug_file_3.close();
   
   for (unsigned int j=0; j<fe_regs.size(); j++){
      if ((fe_regs[j] != fe_rb_regs[j]) && (data_cs == 0)){
         if(j<=9){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 0 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=18){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 1 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=27){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 2 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=36){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 3 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=45){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 4 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=54){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 5 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=64){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 6 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else if(j<=72){
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "FE-ADC 7 SPI failed " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
	 else{
	    WIBException::FEMB_SPI_READBACK_MISMATCH e;
            std::stringstream expstr;
            expstr << " for FEMB: " << int(iFEMB) << "Other SPI failed (except 1,2,3,4,5,6,7) " << j << "  " << fe_regs[j] << "  " << fe_rb_regs[j];
            e.Append(expstr.str().c_str());
            throw e;
	 }
      }
   } // loop over j
   
   WriteFEMB(iFEMB,9,9); // enable data stream to WIB and reset transceiver
   CheckFEMBRegisters(9,9,iFEMB,30);
   //sleep(2);
   //FEMB_UDPACQ_V2(iFEMB-1);
   TLOG_INFO(identification) << "************* CE_CHK_CFG completed ****************" << TLOG_ENDL;
}

void WIB::ConfigFEMB_to_send_fake_data(uint8_t iFEMB, uint8_t fk_mode){
   const std::string identification = "WIB::ConfigFEMB_to_send_fake_data";
   TLOG_INFO(identification) << "************* Now Starting ConfigFEMB_to_send_fake_data  ****************" << TLOG_ENDL;
   if (iFEMB < 1 || iFEMB > 4){
       WIBException::WIB_BAD_ARGS e;
       std::stringstream expstr;
       expstr << "FEMB number should be 1, 2, 3 or 4, but you have provided: "<< int(iFEMB);
       e.Append(expstr.str().c_str());
       throw e;
   }
   
   if (fk_mode < 1 || fk_mode > 4){
       WIBException::WIB_BAD_ARGS e;
       std::stringstream expstr;
       expstr << "Fake data mode should be 1, 2, 3, or 4, but you have provided"<< int(fk_mode);
       e.Append(expstr.str().c_str());
       throw e;
   }
   
   WriteFEMB(iFEMB,"FEMB_TST_SEL",fk_mode);
   CheckFEMBRegisters(fk_mode,"FEMB_TST_SEL",iFEMB,30);
   TLOG_INFO(identification) << "************* ConfigFEMB_to_send_fake_data completed ****************" << TLOG_ENDL;
}
