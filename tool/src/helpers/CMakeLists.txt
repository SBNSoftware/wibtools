cet_add_compiler_flags( -Wimplicit-fallthrough=0 -Werror -Wno-literal-suffix -Wno-non-virtual-dtor)

cet_make(
	LIBRARY_NAME
	WIBTool_Helpers
  LIBRARIES
  )
  
