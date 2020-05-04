#ifndef __MBB_STATUS_HH__
#define __MBB_STATUS_HH__

#include <MBB.hh>
#include <MBBException.hh>
#include <StatusDisplay.hh>

namespace WIBTool{
  class MBBStatus: public StatusDisplay {

  public:
      MBBStatus(MBB *_mbb):mbb(NULL){
      if(_mbb == NULL){
	MBBException::MBB_BAD_ARGS e;
        e.Append("Bad pointer for MBB status table\n");
        throw e;
	}
      mbb = _mbb;
      //SetVersion(mbb->GetSVNVersion());
    }

  private:
    void Process(std::string const & singleTable);        
    void ProcessPTC();
    MBB * mbb;
    
    //uint32_t CRATE_PWR[CRATE_COUNT];   // 0=off, 1=on
    uint32_t CRATE_PULSE_SRC;
    uint32_t PERIOD;                                         
    uint32_t CRATE_PULSE_PERIOD;                                          
    uint32_t CRATE_CLK_TYPE;
    uint32_t CRATE_CLK_STATUS;
    uint32_t FIRMWARE_VER;
    uint32_t FIRMWARE_TRK;
    uint32_t COMPILATION_DT;
    uint32_t COMPILATION_TM;
    uint32_t power;                                                                              
  };

}
#endif
