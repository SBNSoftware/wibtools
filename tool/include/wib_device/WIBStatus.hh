#ifndef __WIB_STATUS_HH__
#define __WIB_STATUS_HH__

#include <WIB.hh>
#include <WIBException.hh>
#include <helpers/StatusDisplay/StatusDisplay.hh>



namespace WIBTool{
  class WIBStatus: public StatusDisplay {
  public:
    WIBStatus(WIB *_wib):wib(NULL){
      if(_wib == NULL){
	WIBException::WIB_BAD_ARGS e;
	e.Append("Bad pointer for WIB status table\n");
	throw e;
      }
      wib = _wib;
      SetVersion(wib->GetSVNVersion());
    }
  private:
    void Process(std::string const & singleTable);
    //void ProcessFEMB(uint8_t iFEMB,std::string const & singleTable);
    void ProcessFEMB(uint8_t iFEMB);
    WIB * wib;

    int   FEMB_PWR[FEMB_COUNT];   // 0=off, 1=on
    float FEMB_TEMP[FEMB_COUNT];  // C
    float FEMB_VCC[FEMB_COUNT];   // V
    float FEMB_V[FEMB_COUNT][6];  // mV
    float FEMB_C[FEMB_COUNT][6];  // mV
  };

}
#endif
