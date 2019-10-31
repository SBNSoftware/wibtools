#ifndef __WIB_STATUS_HH__
#define __WIB_STATUS_HH__

#include <WIB.hh>
#include <WIBException.hh>
#include <StatusDisplay.hh>
#include <utility>
#include <vector>

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
  
    std::map<std::string,double> RetrieveStatusVars();
  
    void Process(std::string const & option);
    void StartPowerMes();
    void ProcessFEMB(uint8_t iFEMB);
    void ProcessWIB();
    void PrintPowerTable();
    void PrintFEMBTable();
    void PrintWIBClockTable();
    uint32_t ConvertSignedInt(uint32_t in);

  protected:
    WIB * wib;
    
    
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
    
    // WIB/FEMB monitoring 
    int     LINK_STATUS[FEMB_COUNT][4];
    int     EQUALIZER_STATUS[FEMB_COUNT][4];
    int     CHKSUM_ERROR_COUNT[FEMB_COUNT][4];
    int     TIME_STAMP[FEMB_COUNT][4];
    int     TS_ERROR_COUNT[FEMB_COUNT][4];
    int     FRAME_ERROR_COUNT[FEMB_COUNT][4];
    int     FEMB_CLK;
    int     FEMB_CMD;
    int     FEMB_INT_CLK; 
    int     PLL_CLK_LOL;
    int     PLL_CLK_LOS; 

    // For the FEMBs we want:
    int     ADC_READOUT_EN[FEMB_COUNT];     // 0x08 bit 4
    int     CLOCK_SWITCH[FEMB_COUNT];       // 0x08 bit 16
    int     ACTIVE_CLOCK[FEMB_COUNT];       // 0x08 bit 31
    int     CLOCK_STATUS[FEMB_COUNT][2];    // 0x08 bit 29(clk1) and bit 30(clk0) -- 0=good, 1=bad
    int     STREAM_EN[FEMB_COUNT];          // 0x09 bit 0
    int     ADC_SEND_EN[FEMB_COUNT];        // 0x09 bit 3
    int     ADC_SYNC_MODE[FEMB_COUNT];      // 0x29 0:1
    int     FEMB_TEST_SEL[FEMB_COUNT];      // 0x2A
    int     FIRMWARE_VER[FEMB_COUNT];       // 0x101 0:15
    int     ID[FEMB_COUNT];                 // 0x101 16:31
    int     COMPILED_VER[FEMB_COUNT];       // 0x102 0:31
    int     DATE_COMPILED[FEMB_COUNT];      // 0x103 0:31 (in HEX)
    int     TIME_COMPILED[FEMB_COUNT];      // 0x104 0:31 (in HEX)

  };

}
#endif
