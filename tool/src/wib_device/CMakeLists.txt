include_directories(../../../exception/include)
include_directories(../../../MBB/include)
include_directories(../../../WIB/include)

cet_make(
	LIBRARY_NAME
	WIBTool_WIBDevice
  LIBRARIES
	ToolException
	MBB
	WIBTool
  )
  
