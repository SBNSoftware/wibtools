include_directories(../../../exception/include)
include_directories(../../../MBB/include)
include_directories(../../../WIB/include)

cet_add_compiler_flags(-Wimplicit-fallthrough=0 -Werror -Wno-literal-suffix -Wno-non-virtual-dtor ) 

cet_make(
	LIBRARY_NAME
	WIBTool_WIBDevice
  LIBRARIES
	ToolException
	MBB
	WIBTool
  )
  
