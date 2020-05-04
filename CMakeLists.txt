# ======================================================================
#  wibtools main build file
#
#  cd ../path/to/build/directory
#  source ../path/to/wibtools/ups/setup_for_development -p 
#  buildtool -bp -j$(nproc)
# ======================================================================

# use cmake 3.2 or later
cmake_minimum_required (VERSION 3.2)

project(wibtools)

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

IF (SANITIZE_ADDRESS)
  string(TOUPPER ${SANITIZE_ADDRESS} SANITIZE_ADDRESS_UC )
  if( ${SANITIZE_ADDRESS_UC} MATCHES "TRUE" )
    MESSAGE (STATUS "Enable  -fsanitize=address")
    cet_add_compiler_flags(-fsanitize=address)
  endif()
ENDIF()


cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_ups_boost  ( v1_70_0 )
find_ups_product( cetbuildtools v7_15_01 )
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
