cet_add_compiler_flags( -Wno-unused -Wno-non-virtual-dtor -Wno-type-limits )

cet_make(
	LIBRARY_NAME
	WIB
  LIBRARIES
	ToolException
	WIBTool_Helpers
	WIBTool_StatusDisplay
	${Boost_REGEX_LIBRARY}
  )
  
