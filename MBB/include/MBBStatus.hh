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
    //void ProcessPTC(uint8_t iFEMB,std::string const & singleTable);        
    void ProcessPTC(uint8_t icrate);
    MBB * mbb;
    
    uint32_t CRATE_PWR[CRATE_COUNT];   // 0=off, 1=on
    uint32_t CRATE_PULSE_SRC;
    uint32_t PERIOD;                                         
    uint32_t CRATE_PULSE_PERIOD;                                          
    uint32_t CRATE_CLK_TYPE;
    uint32_t CRATE_CLK_STATUS;                                       
    uint32_t WIB_PWR[WIB_COUNT];                                       
  };

}
#endif
