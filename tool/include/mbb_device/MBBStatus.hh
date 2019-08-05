#ifndef __MBB_STATUS_HH__
#define __MBB_STATUS_HH__

#include <MBB.hh>
#include <MBBException.hh>
#include <helpers/StatusDisplay/StatusDisplay.hh>
#include <BNL_UDP_Exception.hh>


namespace WIBTool{
  class MBBStatus: public StatusDisplay {
  public:
    MBBStatus(MBB *_mbb):mbb(NULL){ //not sure about whats happening here.
      if(_mbb == NULL){
	MBBException::MBB_BAD_ARGS e;
        e.Append("Bad pointer for MBB status table\n");//not sure.
        throw e;
	}
      mbb = _mbb;
      SetVersion(mbb->GetSVNVersion());//not sure. WIB/include/WIBBase.hh
    }
  private:
    void Process(std::string const & singleTable);
    //void ProcessPTC(uint8_t iFEMB,std::string const & singleTable);        
    void ProcessPTC(uint8_t icrate);
    MBB * mbb;

    
    int   CRATE_PWR[CRATE_COUNT];   // 0=off, 1=on                              
    float CRATE_PULSE_SRC[CRATE_COUNT];                                        
    float CRATE_PULSE_PERIOD[CRATE_COUNT];                                        
    float CRATE_WR_REG[CRATE_COUNT];                                       
    float WIB_PWR[WIB_COUNT];                                       
  };

}
#endif
