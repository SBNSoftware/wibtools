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

    uint32_t ConvertSignedInt(uint32_t in);
    
    int   FEMB_PWR[FEMB_COUNT];   // 0=off, 1=on
    double FEMB_TEMP[FEMB_COUNT];  // C
    double FEMB_VCC[FEMB_COUNT];   // V
    double FEMB_V[FEMB_COUNT][6];  // V
    double FEMB_C[FEMB_COUNT][6];  // mA
  };

}
#endif
