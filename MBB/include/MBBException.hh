#ifndef __MBB_EXCEPTION_HH__
#define __MBB_EXCEPTION_HH__ 1

//#include <MBBException.hh>
#include <MBBException/ExceptionBase.hh>

namespace MBBException{       
  //Exceptions for MBB
   ExceptionClassGenerator(MBB_INDEX_OUT_OF_RANGE,"Index out of range\n")
  //  ExceptionClassGenerator(MBB_FEMB_RANGE,"MBB FEMB out of range\n")
  //  ExceptionClassGenerator(MBB_DAQ_LINK_RANGE,"MBB DAQ Link out of range\n")
  ExceptionClassGenerator(MBB_BUSY,"MBB BUSY\n")
  ExceptionClassGenerator(MBB_FEATURE_NOT_SUPPORTED,"Requested feature is not available\n")
  ExceptionClassGenerator(MBB_BAD_ARGS,"MBB bad args")
  ExceptionClassGenerator(MBB_ERROR,"MBB Error")
  ExceptionClassGenerator(MBB_DAQMODE_UNKNOWN,"Unknown whether MBB is in RCE or FELIX mode\n")
  ExceptionClassGenerator(MBB_FAKE_DATA_ON_HALF_FELIX_LINK,"Fake data mode set on only 1 FEMB of a FELIX link--not allowed")
  ExceptionClassGenerator(MBB_FLASH_TIMEOUT,"A timeout occured on MBB flash")
  ExceptionClassGenerator(MBB_FLASH_ERROR,"An error while using the flash")
  ExceptionClassGenerator(MBB_FLASH_IHEX_ERROR,"An error while parsing intel Hex files")
  ExceptionClassGenerator(MBB_DTS_ERROR,"MBB timing system error")
  
}

#endif
