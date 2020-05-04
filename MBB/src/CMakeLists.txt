include_directories(../../WIB/include)
include_directories(../../exception/include)

cet_add_compiler_flags( -Wimplicit-fallthrough=0 -Werror -Wno-literal-suffix -Wno-unused -Wno-type-limits -Wno-non-virtual-dtor)

cet_make(
	LIBRARY_NAME
	MBB
  LIBRARIES
	ToolException
	WIB
  )
  
