#include <wib_device/WIBDevice.hh>

WIBTool::WIBDevice::WIBDevice(std::vector<std::string> arg)
    : CommandList<WIBDevice>("WIB"),
      wib(NULL){    
    LoadCommandList();
    if (arg.size() < 1){
      WIBException::DEVICE_CREATION_ERROR e;
      e.Append("Bad argument count");
      throw e;
    }
    //Get address
    Address=arg[0];    
    //Set WIB address table
    if(arg.size() > 1){
      //argument
      WIBTable = arg[1];
    }else{
      //default
      WIBTable = "WIB.adt";
    }

    //Set WIB address table
    if(arg.size() > 2){
      //argument
      FEMBTable = arg[2];
    }else{
      //default
      FEMBTable = "FEMB.adt";
    }


    wib = new WIB(Address,WIBTable,FEMBTable);    
    SetInfo(wib->GetAddress().c_str());
  }

WIBTool::WIBDevice::~WIBDevice(){
    if(wib != NULL){
      delete wib;
    }
  }


void WIBTool::WIBDevice::LoadCommandList(){
  AddCommand("udptest",&WIBDevice::TestUDP,
	     "Test udp link\n"			\
	     "  Usage:\n"			\
	     "  udptest N\n");
  AddCommand("read",&WIBDevice::Read,
	     "Read from WIB\n"			\
	     "  Usage:\n"			\
	     "  read <address>\n",
	     &WIBDevice::autoComplete_WIBAddressTable);
  AddCommandAlias("rd","read");
  AddCommand("write",&WIBDevice::Write,
	     "Write to WIB\n"			\
	     "  Usage:\n"			\
	     "  write <address> <value>\n",
	     &WIBDevice::autoComplete_WIBAddressTable);
  AddCommandAlias("wr","write");
  AddCommand("names",&WIBDevice::Names,
	     "List matching address table names."\
	     "  Usage:\n"\
	     "  names <regex>\n",
	     &WIBDevice::autoComplete_WIBAddressTable);
  AddCommandAlias("nodes","names");
  AddCommand("addrs",&WIBDevice::Addresses,
	     "List matching WIB address table names to numerical address."\
	     "  Usage:\n"\
	     "  addrs <address> <address upper bound>\n"
	     );
  AddCommand("faddrs",&WIBDevice::FEMBAddresses,
	     "List matching FEMB address table names to numerical address."\
	     "  Usage:\n"\
	     "  faddrs <address> <address upper bound>\n"
	     );

  AddCommand("femb_power",&WIBDevice::PowerFEMB,
	     "Power on/off FEMB"\
	     "  Usage:\n"\
	     "  femb_power <iFEMB> <on|off>\n"
	     );
  AddCommandAlias("fpwr","femb_power");


  AddCommand("fread",&WIBDevice::FEMBRead,
	     "Read from FEMB\n"			\
	     "  Usage:\n"			\
	     "  fread <FEMB> <address>\n",
	     &WIBDevice::autoComplete_FEMBNumber_FEMBAddressTable);
  AddCommand("ADC_enable",&WIBDevice::EnableADC,
	     "Turn on/off ADCs\n"		\
	     "  Usage:\n"			\
	     "  ADC_enable <FEMB> <1:on/0:off>\n"
	     );
  AddCommandAlias("frd","fread");
  AddCommand("fwrite",&WIBDevice::FEMBWrite,
	     "Write to FEMB\n"			\
	     "  Usage:\n"			\
	     "  fwrite <FEMB> <address> <value>\n",
	     &WIBDevice::autoComplete_FEMBNumber_FEMBAddressTable);
  AddCommandAlias("fwr","fwrite");
  AddCommand("FEMBnames",&WIBDevice::FEMBNames,
	     "List matching address table names for FEMBs."\
	     "  Usage:\n"\
	     "  FEMBnames <regex>\n",
	     &WIBDevice::autoComplete_FEMBAddressTable);
  AddCommandAlias("fnames","FEMBnames");
  AddCommandAlias("fnodes","fnames");

  AddCommand("rdcds",&WIBDevice::Read_DTS_CDS_I2C,
	     "Read from DTS CDS i2c"\
	     "  Usage:\n"\
	     "  rdcds address <byte_count>\n"
	     );
  AddCommand("rdsi5344",&WIBDevice::Read_DTS_SI5344_I2C,
	     "Read from DTS Si5344 i2c"		\
	     "  Usage:\n"\
	     "  rdsi5344 address value <byte_count>\n"
	     );
  AddCommand("wrcds",&WIBDevice::Write_DTS_CDS_I2C,
	     "Write to DTS CDS i2c"\
	     "  Usage:\n"\
	     "  wrcds address value <byte_count>\n"
	     );
  AddCommand("wrsi5344",&WIBDevice::Write_DTS_SI5344_I2C,
	     "Write to DTS Si5344 i2c"\
	     "  Usage:\n"\
	     "  wrsi5344 address value <byte_count>\n"
	     );
  AddCommand("flash_wr",&WIBDevice::WriteFlash,
	     "Write hexout file to the WIB flash\n"\
	     "  Usage:\n"\
	     "  flash_wr filename.hexout\n");
  AddCommand("flash_rd",&WIBDevice::ReadFlash,
	     "Write WIB flash contents to a file\n"\
	     "  Usage:\n"\
	     "  flash_rd filename\n");
  AddCommand("lflash_wr",&WIBDevice::WriteLocalFlash,
	     "Write to the local flash\n"\
	     "  Usage:\n"\
	     "  lflash_wr address data\n");
  AddCommandAlias("lfwr","lflash_wr");
  AddCommand("lflash_rd",&WIBDevice::ReadLocalFlash,
	     "Read from local flash\n"\
	     "  Usage:\n"\
	     "  lflash_rd address\n");
  AddCommandAlias("lfrd","lflash_rd");
  AddCommand("spy_daq_ev",&WIBDevice::ReadDAQLinkSpyBufferEvents,
	     "Capture stream from DAQ Link and decode events\n"\
	     "  Usage:\n"\
	     "  spy_daq_ev link <mode> <filename>\n");
  AddCommand("spy_daq",&WIBDevice::ReadDAQLinkSpyBuffer,
	     "Capture stream from DAQ Link\n"\
	     "  Usage:\n"\
	     "  spy_daq link <mode> <filename>\n");
  AddCommand("en_dlink",&WIBDevice::EnableDAQLink,
	     "Enable event builder on DAQ link\n"\
	     "  Usage:\n"\
	     "  en_dlink link\n");
  AddCommand("en_dlink_lite",&WIBDevice::EnableDAQLink_Lite,
	     "Enable event builder on DAQ link\n"\
	     "  Usage:\n"\
	     "  en_dlink_lite <link> <enable=1>\n");
  AddCommand("configWIBFakeData",&WIBDevice::ConfigWIBFakeData,
	     "Enable/disable WIB fake data from the FEMBs.\n"	\
	     "  Usage:\n"					\
	     "  configWIBFakeData <femb1> <femb2> <femb3> <femb4> [<counter>]\n"                 \
         "    where femb1/2/3/4 are 0 for real data and 1 for fake, \n"                      \
         "    and counter is 0 for a counter in the samples of the \n"                       \
         "    COLDATA frame (default) and 1 for a counter instead of the COLDATA frame");
  AddCommand("FEMBStrSrc",&WIBDevice::SetFEMBStreamSource,
	     "Set source of FEMB stream to captured data (R) or fake (F).\n"	\
	     "  Usage:\n"					\
	     "  FEMBStrSrc iFEMB iStream R/F\n");
  AddCommand("FEMBSource",&WIBDevice::FEMBSource,
	     "Set display of FEMB source to captured data (R) or fake (F).\n"	\
	     "  Usage:\n"					\
	     "  FEMBSource iFEMB R/F\n");
  AddCommand("FEMBStrMode",&WIBDevice::SetFEMBFakeData,
	     "Set FEMB fake data formatting counter or samples (C/S)\n"	\
	     "  Usage:\n"					\
	     "  FEMBStrMode iFEMB iCD C/S\n");
  AddCommand("status",&WIBDevice::StatusDisplay,
	     "Show status display\n"	\
	     "  Usage:\n"					\
	     "  status <level> <table>\n",
	     &WIBDevice::autoComplete_WIBTables);
  AddCommand("html-status",&WIBDevice::StatusDisplayHTML,
	     "Write status display to status.html\n"	\
	     "  Usage:\n"					\
	     "  html-status <level> <table>\n");
  AddCommand("file-status",&WIBDevice::StatusDisplayFILE,
	     "Write status display to status-<date>.dump\n"	\
	     "  Usage:\n"					\
	     "  file-status <level> <table>\n");
  AddCommand("dump",&WIBDevice::DumpAddressTable,
	     "Dump full address table to RegDump-<date>.txt\nIf argument is \"stdout\", then dump to stdout"	\
	     "  Usage:\n"					\
	     "  dump [stdout]\n");
  AddCommand("dumpFEMB",&WIBDevice::DumpFEMBAddressTable,
	     "Dump full address table of FEMB to FEMBRegDump-<date>.txt\nIf argument is \"stdout\", then dump to stdout"	\
	     "  Usage:\n"					\
	     "  dumpFEMB <iFEMB 1-4> [stdout]\n");
  AddCommand("configSI5344",&WIBDevice::Write_DTS_SI5344_Config,
	     "Write configuration file to the SI5344\n"	\
	     "  Usage:\n"					\
	     "  confSI5344 filename\n");
  AddCommand("getSI5344_Page",&WIBDevice::Read_DTS_SI5344_Page,
	     "Get the current SI5344 page.\n"	\
	     "  Usage:\n"					\
	     "  getSI5344_Page\n");
  AddCommand("setSI5344_Page",&WIBDevice::Write_DTS_SI5344_Page,
	     "Set the current SI5344 page.\n"	\
	     "  Usage:\n"					\
	     "  setSI5344_Page page_number\n");
  AddCommand("configDTSCDS",&WIBDevice::ConfigureDTSCDS,
	     "Configure the DTS CDS.\n"	\
	     "  Usage:\n"					\
	     "  configDTSCDS <input>\n");
  AddCommand("resetSI5344",&WIBDevice::ResetSI5344,
	     "Reset the Si5344.\n"				\
	     "  Usage:\n"					\
	     "  resetSi5344 \n");
  AddCommand("selectSI5344",&WIBDevice::SelectSI5344,
	     "Enable and use the SI5344.\n"			\
	     "  Usage:\n"					\
	     "  selectSI5344 <input:1,2,3> <1:on/0:off>\n");
  AddCommand("selectSI5342",&WIBDevice::SelectSI5342,
	     "Enable and use the SI5342.\n"			\
	     "  Usage:\n"					\
	     "  selectSI5342 <input:1,2,3> <1:on/0:off>\n");
  AddCommand("initDTS",&WIBDevice::InitializeDTS,
	     "Initialize the DTS with source FP(1) or BP(0).\n" \
	     "  Usage:\n"					\
	     "  initDTS <PDTS source> <clockSource> <PDTSAlignment_timeout>\n");
  AddCommand("resetWIB",&WIBDevice::ResetWIB,
	     "Reset the WIB.\n"				        \
	     "  Usage:\n"					\
	     "  resetWIB <force UDP Reset>\n");
  AddCommand("resetPDTS",&WIBDevice::ResetPDTS,
	     "Simple reset of the PDTS\n"				\
	     "  Usage:\n"					\
	     "  resetPDTS \n");
  AddCommand("resetWIBAndCfgDTS",&WIBDevice::ResetWIBAndCfgDTS,
	     "Reset WIB and configure timing."				\
	     " Afterward, you can configure (fake) FEMBs and then start streaming to DAQ\n"				\
	     "  Usage:\n"					\
	     "  resetWIBAndCfgDTS useLocal <PDTS TGRP (0-15) (required if useLocal=0)> <PDTS source (default:0 backplane, 1 frontpanel)> <PDTSAlignment_timeout>\n");
  AddCommand("checkedResetWIBAndCfgDTS",&WIBDevice::CheckedResetWIBAndCfgDTS,
	     "Reset WIB and configure timing."				\
	     " Afterward, you can configure (fake) FEMBs and then start streaming to DAQ\n"				\
	     "  Usage:\n"					\
	     "  checkedResetWIBAndCfgDTS useLocal <PDTS TGRP (0-15) (required if useLocal=0)> <PDTS source (default:0 backplane, 1 frontpanel)> <PDTSAlignment_timeout>\n");
  AddCommand("startStreamToDAQ",&WIBDevice::StartStreamToDAQ,
	     "Start streaming to DAQ."				\
	     " Beforehand, you should have reset the WIB, configured DTS, and setup the (fake) FEMBs.\n"				\
	     "  Usage:\n"					\
	     "  startStreamToDAQ\n");


  AddCommand("rdsi5342",&WIBDevice::Read_DAQ_SI5342_I2C,
	     "Read from DAQ Si5342 i2c"		\
	     "  Usage:\n"\
	     "  rdsi5342 address value <byte_count>\n"
	     );
  AddCommand("wrsi5342",&WIBDevice::Write_DAQ_SI5342_I2C,
	     "Write to DAQ Si5342 i2c"\
	     "  Usage:\n"\
	     "  wrsi5342 address value <byte_count>\n"
	     );
  AddCommand("configSI5342",&WIBDevice::Write_DAQ_SI5342_Config,
	     "Write configuration file to the SI5342\n"	\
	     "  Usage:\n"					\
	     "  confSI5342 filename\n");
  AddCommand("getSI5342_Page",&WIBDevice::Read_DAQ_SI5342_Page,
	     "Get the current SI5342 page.\n"	\
	     "  Usage:\n"					\
	     "  getSI5342_Page\n");
  AddCommand("setSI5342_Page",&WIBDevice::Write_DAQ_SI5342_Page,
	     "Set the current SI5342 page.\n"	\
	     "  Usage:\n"					\
	     "  setSI5342_Page page_number\n");
  AddCommand("resetSI5342",&WIBDevice::ResetSI5342,
	     "Reset the Si5342.\n"				\
	     "  Usage:\n"					\
	     "  resetSi5342 \n");
  AddCommand("start_sync_DTS",&WIBDevice::DTSStartSync,
	     "Start sync with DTS\n"				\
	     "  Usage:\n"					\
	     "  start_sync_DTS \n");
  AddCommand("spy_femb",&WIBDevice::ReadCDLinkSpyBuffer,
	     "Read out FEMB spy buffer\n"				\
	     "  Usage:\n"					\
	     "  spy_femb <outfile> \n");
  AddCommand("configFEMB",&WIBDevice::ConfigFEMB,
	     "Configure an FEMB\n"				\
	     "  Usage:\n"					\
	     "  configFEMB <iFEMB 1-4> <Gain 0-3> <Shaping Time 0-3> <High Baseline 0-1> <High Leakage 0-1> <Leakage x10 0-1> <AC coupling 0-1> <Buffer 0-1> <Use External Clock 0-1> <Pulser Mode 0-2> <Pulser Amplitude 0-63> [<Clock Phase 0x000-0xFFFF>]\n");
  AddCommand("configFEMBFakeData",&WIBDevice::ConfigFEMBFakeData,
	     "Configure an FEMB in fake data mode\n"				\
	     "  Usage:\n"					\
	     "  configFEMBFakeData <iFEMB 1-4> <mode 1-4> [<fake word 0x0-0xFFF>]\n" \
         "    where fake mode is 1 for fake word, 2 for fake counter (hardcoded waveform of counter), \n" \
         "    3 for fake FEMB and channel ID, 4 for fake counter and channel ID");
  AddCommand("configFEMBMode",&WIBDevice::ConfigFEMBMode,
	     "Configure an FEMB for pulsing\n"				\
	     "  Usage:\n"					\
	     "  configFEMBMode <iFEMB 1-4> <pls_cs 0-2> <dac_sel>  <fpga_dac>  <asic_dac> < mon_cs>\n");
  AddCommand("setupFEMBExtClock",&WIBDevice::SetupFEMBExtClock,
	     "Setup the external clock for an FEMB\n"				\
	     "  Usage:\n"					\
	     "  setupFEMBExtClock <iFEMB 1-4>\n");
  AddCommand("setupFEMBExtClock",&WIBDevice::SetupFEMBExtClock,
	     "Setup the external clock for an FEMB\n"				\
	     "  Usage:\n"					\
	     "  setupFEMBExtClock <iFEMB 1-4>\n");
  AddCommand("setupFPGAPulser",&WIBDevice::SetupFPGAPulser,
	     "Setup the Pulser on the FPGA\n"				\
	     "  Usage:\n"					\
             "  setupFPGAPulser <iFEMB 1-4> <dac_val>\n");
  AddCommand("setupASICPulserBits",&WIBDevice::SetupASICPulserBits,
	     "Setup the pulser bits on the ASICS\n"				\
	     "  Usage:\n"					\
             "  setupFPGAPulser <iFEMB 1-4>\n");
  AddCommand("setupFEMBASICs",&WIBDevice::SetupFEMBASICs,
	     "Setup the ASICs for an FEMB\n"				\
	     "  Usage:\n"					\
	     "  setupFEMBASICs <iFEMB 1-4> [<gain 0-3> <shape 0-3> <highBaseline 0-1> <highLeakage 0-1> <highLeakage10 0-1> <acCoupling 0-1> <buffer 0-1> <useExtClock 0-1> <internalDACControl 0-2> <internalDACValue 0-63>]\n");
  AddCommand("rdQSFP",&WIBDevice::Read_QSFP_I2C,
	     "Read from QSFP i2c"		\
	     "  Usage:\n"\
	     "  rdQSFP address value <byte_count>\n"
	     );
  AddCommand("wrQSFP",&WIBDevice::Write_QSFP_I2C,
	     "Write to QSFP i2c"\
	     "  Usage:\n"\
	     "  rdWSFP address value <byte_count>\n"
	     );
  AddCommand("pdtsHist",&WIBDevice::PrintPDTSERRHistory,
	     "Print PDTS debug history after an error"\
	     "  Usage:\n"\
	     "  pdtsHist\n"
	     );
  AddCommand("pdtsRunning",&WIBDevice::PDTSInRunningState,
	     "Check if the PDTS is in the RUN state.\n"				\
	     "  Usage:\n"					\
	     "  pdtsRunning \n");


}

