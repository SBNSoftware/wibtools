# ======================================================================
#  wibtools main build file
#
#  cd ../path/to/build/directory
#  source ../path/to/wibtools/ups/setup_for_development -p 
#  buildtool -bp -j$(nproc)
# ======================================================================



cmake_minimum_required (VERSION 3.16)

find_package(cetbuildtools)

project(wibtools VERSION 0.04.00)

SET ( CETBUILDTOOLS_VERSION $ENV{CETBUILDTOOLS_VERSION} )
IF (NOT CETBUILDTOOLS_VERSION)
    MESSAGE (FATAL_ERROR "ERROR: setup cetbuildtools to get the cmake modules")
ENDIF()

set(CMAKE_MODULE_PATH
    $ENV{CETBUILDTOOLS_DIR}/Modules
    ${CMAKE_MODULE_PATH})

include(CetCMakeEnv)

cet_cmake_env()

cet_set_compiler_flags(DIAGS VIGILANT WERROR NO_UNDEFINED EXTRA_FLAGS )

cet_add_compiler_flags( -Wimplicit-fallthrough=0 )
cet_add_compiler_flags( -Wno-literal-suffix )
cet_add_compiler_flags( -Wno-unused )
cet_add_compiler_flags( -Wno-type-limits )
cet_add_compiler_flags( -Wno-non-virtual-dtor )

IF (SANITIZE_ADDRESS)
  string(TOUPPER ${SANITIZE_ADDRESS} SANITIZE_ADDRESS_UC )
  if( ${SANITIZE_ADDRESS_UC} MATCHES "TRUE" )
    MESSAGE (STATUS "Enable  -fsanitize=address")
    cet_add_compiler_flags(-fsanitize=address)
  endif()
ENDIF()

#exception stack trace
if(DEFINED ENV{USE_EXCEPTIONSTACKTRACE})
        if($ENV{USE_EXCEPTIONSTACKTRACE} GREATER 0)
                message("!!! Exception StackTrace Activated!!!")
                add_definitions(-D EXCEPTIONSTACKTRACE)
                set( EXCEPTIONSTACKTRACE_LIB  artdaq-core_Utilities_ExceptionStackTrace )
                cet_add_compiler_flags( -rdynamic )
        endif()
endif()

cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_boost  ( v1_70_0 )
find_ups_product( TRACE v3_16_02 )
find_ups_product( cetbuildtools v7_17_01 )
find_ups_product( cetpkgsupport v1_14_01 )


# source
add_subdirectory(exception)
add_subdirectory(WIB)
add_subdirectory(MBB)
add_subdirectory(tool)

# ups - table and config files
add_subdirectory(ups)

# packaging utility
include(UseCPack)
