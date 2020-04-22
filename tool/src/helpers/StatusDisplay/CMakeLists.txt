include_directories(../../../../exception/include)

cet_add_compiler_flags(-Werror -Wno-literal-suffix -Wno-non-virtual-dtor ) 

cet_make(
	LIBRARY_NAME
	WIBTool_StatusDisplay
  LIBRARIES
	ToolException
  )
  
