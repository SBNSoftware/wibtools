include_directories(../../../exception/include)
include_directories(../../../MBB/include)
include_directories(../../../WIB/include)

find_program(BASH bash HINTS /bin)
get_cmake_property(_variableNames VARIABLES)

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" WIBTOOLS_VERSION_LIST "${wibtools_VERSION}")
list(GET WIBTOOLS_VERSION_LIST 0 WIBTOOLS_MAJOR)
list(GET WIBTOOLS_VERSION_LIST 1 WIBTOOLS_MINOR)
list(GET WIBTOOLS_VERSION_LIST 2 WIBTOOLS_PATCH)


add_custom_command(
	PRE_BUILD
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc
	COMMAND  ${BASH} -c  "echo -e '#include <WIBBase.hh>' > ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc"
	COMMAND  ${BASH} -c  "echo -e '#include <string>' >> ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc"
	COMMAND  ${BASH} -c  "echo -e 'const int WIBBase::Version(std::stoi(\"${WIBTOOLS_MAJOR}${WIBTOOLS_MINOR}${WIBTOOLS_PATCH}\"));' >>	${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc"
	VERBATIM 
 	)

cet_make_library(LIBRARY_NAME 
	WIBBase_version
  SOURCE
  ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc
	LIBRARIES
	wibtools::ToolException
	Boost::boost
	-lreadline
	-ldl
	)

cet_make(
	LIBRARY_NAME
	WIBTool
  LIBRARIES
  WIB
	wibtools::ToolException
	wibtools::WIBBase_version
	-lreadline
  -lpthread
  )
  
cet_make_exec(NAME WIBTool.exe
	SOURCE
	WIBTool.cxx
  LIBRARIES
	wibtools::ToolException
	wibtools::WIBTool
        wibtools::WIBTool_WIBDevice
        wibtools::WIBTool_MBBDevice
	-lreadline
  )
  
