include_directories(../../WIB/include)
include_directories(../../MBB/include)
include_directories(../../exception/include)

cet_make(
	LIBRARY_NAME
	MBB
  LIBRARIES
	ToolException
	WIB
  )
  
