include_directories(../../../exception/include)
include_directories(../../../MBB/include)
include_directories(../../../WIB/include)

cet_add_compiler_flags(-Wimplicit-fallthrough=0 -Werror -Wno-literal-suffix -Wno-non-virtual-dtor ) 

find_program(BASH bash HINTS /bin)

string(REGEX MATCHALL "[0-9][0-9]?[0-9]?" WIBTOOLS_VERSION_LIST "$ENV{CETPKG_VERSION}")
list(GET WIBTOOLS_VERSION_LIST 0 WIBTOOLS_MAJOR)
list(GET WIBTOOLS_VERSION_LIST 1 WIBTOOLS_MINOR)
list(GET WIBTOOLS_VERSION_LIST 2 WIBTOOLS_PATCH)

add_custom_command(
	PRE_BUILD
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc
	COMMAND  ${BASH} -c  "echo \"#include <WIBBase.hh>\" > ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc"
	COMMAND  ${BASH} -c  "echo \"const int WIBBase::Version(${WIBTOOLS_MAJOR}${WIBTOOLS_MINOR}${WIBTOOLS_PATCH});\" >> ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc"
	VERBATIM 
 	)

add_library(WIBBase_version STATIC ${CMAKE_CURRENT_BINARY_DIR}/WIBBase_version.cc)


cet_make(
	LIBRARY_NAME
	WIBTool
  LIBRARIES
	ToolException
	WIBBase_version
	-lreadline
	-ldl
  )
  
cet_make_exec(WIBTool.exe
	SOURCE
	WIBTool.cxx
  LIBRARIES
	ToolException
	WIBTool
	-lreadline
  )
  
