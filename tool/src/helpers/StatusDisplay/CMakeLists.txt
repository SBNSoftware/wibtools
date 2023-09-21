cet_make_library(
	LIBRARY_NAME
	WIBTool_StatusDisplay
	SOURCE
	StatusDisplay.cc
	StatusDisplayCell.cc
	StatusDisplayMatrix.cc
  LIBRARIES
	wibtools::ToolException
	Boost::boost
  )
  target_include_directories(WIBTool_StatusDisplay PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../include>)
  
