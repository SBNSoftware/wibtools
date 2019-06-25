#ifndef __MMB_EXCEPTION_HH__
#define __MMB_EXCEPTION_HH__ 1

#include <MMBException/ExceptionBase.hh>

namespace MMBException
{       
  //Exceptions for MMB
  ExceptionClassGenerator(MMB_INDEX_OUT_OF_RANGE,"Index out of range\n")
  //  ExceptionClassGenerator(MMB_FEMB_RANGE,"MMB FEMB out of range\n")
  //  ExceptionClassGenerator(MMB_DAQ_LINK_RANGE,"MMB DAQ Link out of range\n")
  ExceptionClassGenerator(MMB_BUSY,"MMB BUSY\n")
  ExceptionClassGenerator(MMB_FEATURE_NOT_SUPPORTED,"Requested feature is not available\n")
  ExceptionClassGenerator(MMB_BAD_ARGS,"MMB bad args")
  ExceptionClassGenerator(MMB_ERROR,"MMB Error")
  ExceptionClassGenerator(MMB_DAQMODE_UNKNOWN,"Unknown whether MMB is in RCE or FELIX mode\n")
  ExceptionClassGenerator(MMB_FAKE_DATA_ON_HALF_FELIX_LINK,"Fake data mode set on only 1 FEMB of a FELIX link--not allowed")
  ExceptionClassGenerator(MMB_FLASH_TIMEOUT,"A timeout occured on MMB flash")
  ExceptionClassGenerator(MMB_FLASH_ERROR,"An error while using the flash")
  ExceptionClassGenerator(MMB_FLASH_IHEX_ERROR,"An error while parsing intel Hex files")
  ExceptionClassGenerator(MMB_DTS_ERROR,"MMB timing system error")

}

#endif
