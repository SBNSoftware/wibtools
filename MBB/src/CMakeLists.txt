
cet_make_library(
	LIBRARY_NAME
	MBB
	SOURCE
	MBB.cc
	MBBStatus.cc
  LIBRARIES
	ToolException
	WIB
  )
  target_include_directories(MBB PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)  
