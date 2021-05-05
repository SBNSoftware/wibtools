cet_add_compiler_flags( -Wno-unused -Wno-non-virtual-dtor -Wno-type-limits -Wno-format-truncation )

include_directories( ../include )

cet_make(
	LIBRARY_NAME
	WIB
  LIBRARIES
	ToolException
	WIBTool_Helpers
	WIBTool_StatusDisplay
	${Boost_REGEX_LIBRARY}
  )
  
