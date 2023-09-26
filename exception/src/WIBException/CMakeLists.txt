cet_make_library(
	LIBRARY_NAME
	ToolException
	SOURCE
	ExceptionBase.cc
	ExceptionBase_stacktrace.cc
  LIBRARIES
  )
  target_include_directories(ToolException PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>)
  
