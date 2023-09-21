include_directories(../../../../exception/include)

cet_make(
	LIBRARY_NAME
	WIBTool_StatusDisplay
  LIBRARIES
	wibtools::ToolException
	Boost::boost
  )
  
