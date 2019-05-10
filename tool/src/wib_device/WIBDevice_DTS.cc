#include <wib_device/WIBDevice.hh>


CommandReturn::status WIBTool::WIBDevice::ResetPDTS(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  //hold the PDTS in reset
  wib->Write("DTS.PDTS_ENABLE",0);
  //Reset the reset request counters
  wib->Write("DTS.PDTS_RESET_REQUESTS",1);
  wib->Write("DTS.PDTS_RESETS_PERFORMED",1);
  //re-enable the PDTS
  wib->Write("DTS.PDTS_ENABLE",1);
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::DTSStartSync(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  wib->StartSyncDTS();

  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::PrintPDTSERRHistory(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  std::vector<uint32_t> data = wib->CaptureHistory("DTS.PDTS_HISTORY_DEWIBG");
  char header[] = "type sfp_los_ok cdr_ok rxph_locked_i rx_err_i rdy_i state  cntr\n";
  char pdata[]  = "%u    %u          %u        %u           %u        %u     0x%X    0x%04X\n";

  if(data.size() > 0){
    printf(header);
    for(size_t iSample = 0; iSample < data.size();iSample++){
      uint32_t val = data[iSample];
      printf(pdata,
	     (val>>1)&0x1,
	     (val>>4)&0x1,
	     (val>>5)&0x1,
	     (val>>6)&0x1,
	     (val>>7)&0x1,
	     (val>>8)&0x1,
	     (val>>9)&0xF,
	     (val>>13)&0xFFFF
	     );
    }
  }else{
    printf("No history\n");
  }
  return CommandReturn::OK;
}

CommandReturn::status WIBTool::WIBDevice::PDTSInRunningState(std::vector<std::string> /*strArg*/,std::vector<uint64_t> /*intArg*/){
  wib->PDTSInRunningState();
  return CommandReturn::OK;
}
