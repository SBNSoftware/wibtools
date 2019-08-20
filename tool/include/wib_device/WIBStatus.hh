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
    void StartPowerMes();
    void ProcessFEMB(uint8_t iFEMB);
    void ProcessWIB();
    WIB * wib;

    uint32_t ConvertSignedInt(uint32_t in);
    
    // Power monitoring
    int     FEMB_PWR[FEMB_COUNT];   // 0=off, 1=on
    double  FEMB_TEMP[FEMB_COUNT];  // C
    double  FEMB_VCC[FEMB_COUNT];   // V
    double  FEMB_V[FEMB_COUNT][6];  // V
    double  FEMB_C[FEMB_COUNT][6];  // mA
    double  WIB_VCC;
    double  WIB_V[4];
    double  WIB_C[4];
    double  WIB_TEMP;
    
    // WIB monitoring parameters
    int     LINK_STATUS[FEMB_COUNT][4];
    int     CHKSUM_ERR[FEMB_COUNT];
    int     TS_COUNT[FEMB_COUNT];
    int     TS_ERROR[FEMB_COUNT];
    int     FRAME_ERROR[FEMB_COUNT];
    int     EQUALIZER_STATUS[FEMB_COUNT][4];

    // For the FEMBs we want:
    //  - everything in 0x08
    //  - 0x09 (bits 0 and 3)
    //  - 0x29
    //  - 0x2A
    //  - 0x101-0x104

  };

}
#endif
