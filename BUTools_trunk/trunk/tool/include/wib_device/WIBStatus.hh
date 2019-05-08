#ifndef __WIB_STATUS_HH__
#define __WIB_STATUS_HH__

#include <WIB/WIB.hh>
#include <WIB/WIBException.hh>
#include <helpers/StatusDisplay/StatusDisplay.hh>

namespace BUTool{
  class WIBStatus: public StatusDisplay {
  public:
    WIBStatus(WIB *_wib):wib(NULL){
      if(_wib == NULL){
	BUException::WIB_BAD_ARGS e;
	e.Append("Bad pointer for WIB status table\n");
	throw e;
      }
      wib = _wib;
      SetVersion(wib->GetSVNVersion());
    }
  private:
    void Process(std::string const & singleTable);
    void ProcessFEMB(uint8_t iFEMB,std::string const & singleTable);
    WIB * wib;
  };

}
#endif
