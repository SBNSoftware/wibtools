
include_directories($ENV{MESSAGEFACILITY_INC})
include_directories($ENV{CETLIB_INC})
include_directories($ENV{CETLIB_EXCEPT_INC})

cet_make_library(
	LIBRARY_NAME
	WIB
	SOURCE
	ADC_ASIC_reg_mapping.cc
	AddressTable.cc
	AddressTable_fileIO.cc 
	AddressTable_IO.cc 
	AddressTable_item.cc 
	AddressTable_search.cc 
	ASIC_reg_mapping.cc 
	BNL_UDP.cc 
	conversions.cc 
	FE_ASIC_reg_mapping.cc 
	WIBBase.cc 
	WIB.cc 
	WIB_CDS.cc 
	WIB_FAKE_CD.cc 
	WIB_FEMB.cc 
	WIB_Flash.cc 
	WIB_History.cc 
	WIB_localFlash.cc 
	WIB_QSFP.cc 
	WIB_SI5342.cc 
	WIB_SI5344.cc 
	WIB_spybuffer.cc 
	WIBStatus.cc
  LIBRARIES PUBLIC
	wibtools::ToolException
	wibtools::WIBTool_Helpers
	wibtools::WIBTool_StatusDisplay
  messagefacility::MF_MessageLogger
	Boost::regex
	TRACE::TRACE
  )
  
  target_include_directories(WIB PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
