cet_make_library(
	LIBRARY_NAME
	WIBTool_Helpers
	SOURCE
	parseHelperss.cc
	register_helper.cc
  LIBRARIES
	Boost::boost
  )
  target_include_directories(WIBTool_Helpers PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>)
  
