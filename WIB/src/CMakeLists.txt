include_directories( ../include )

cet_make(
	LIBRARY_NAME
	WIB
  LIBRARIES PUBLIC
	wibtools::ToolException
	wibtools::WIBTool_Helpers
	wibtools::WIBTool_StatusDisplay
	Boost::regex
	TRACE::TRACE
  )
  
