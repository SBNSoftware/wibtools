# ======================================================================
#  wibtools main build file
#
#  cd ../path/to/build/directory
#  source ../path/to/wibtools/ups/setup_for_development -p
#  buildtool -bp -j$(nproc)
# ======================================================================


cmake_minimum_required (VERSION 3.18.2)

find_package(cetmodules)

project(wibtools VERSION 1.09.01)

include(CetCMakeEnv)
cet_cmake_env()

cet_cmake_module_directories(Modules BINARY)
include(InstallOther)

cet_set_compiler_flags(DIAGS VIGILANT
  WERROR
  NO_UNDEFINED
  EXTRA_FLAGS
)

string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE_UC)
if(BTYPE_UC STREQUAL DEBUG)
  add_definitions(-DDEBUGME)
  if(DEFINED ENV{USE_ASAN})
    if($ENV{USE_ASAN} GREATER 0)
      message("!!! Address Sanitizer Activated!!!")
      add_compile_options(-fsanitize=address -Wno-unused-command-line-argument)
      add_link_options(-lasan)
    endif()
  endif()
  if(DEFINED ENV{USE_TSAN})
    if($ENV{USE_TSAN} GREATER 0)
      message("!!! Thread Sanitizer Activated!!!")
      add_compile_options(-fsanitize=thread -Wno-unused-command-line-argument)
      add_link_options(-ltsan)
    endif()
  endif()
  if(DEFINED ENV{USE_GCOV})
    add_compile_options(-fprofile-arcs -ftest-coverage --coverage)
    add_link_options(--coverage)
  endif()
endif()

#exeption stack trace
if(DEFINED ENV{USE_EXCEPTIONSTACKTRACE})
  if($ENV{USE_EXCEPTIONSTACKTRACE} GREATER 0)
    message("!!! Exception StackTrace Activated!!!")
    add_definitions(-D EXCEPTIONSTACKTRACE)
    set( EXCEPTIONSTACKTRACE_LIB  artdaq-core_Utilities_ExceptionStackTrace )
    add_compile_options( -rdynamic -Wno-unused-command-line-argument )
    add_link_options( -rdynamic )
  endif()
endif()


cet_add_compiler_flags( -Wimplicit-fallthrough=0 )
cet_add_compiler_flags( -Wno-literal-suffix )
cet_add_compiler_flags( -Wno-unused )
cet_add_compiler_flags( -Wno-type-limits )
cet_add_compiler_flags( -Wno-non-virtual-dtor )

#cet_report_compiler_flags()

# these are minimum required versions, not the actual product versions
find_package(Boost 1.75.0 COMPONENTS regex QUIET)
find_package( TRACE )
find_package( cetpkgsupport )
find_package( messagefacility 2.08.04 REQUIRED EXPORT)
find_package( cetlib 3.13.04  REQUIRED EXPORT)
find_package( cetlib_except  1.07.04 REQUIRED EXPORT)
#find_package( artdaq 3.12.05 REQUIRED EXPORT)


# source
add_subdirectory(exception)
add_subdirectory(WIB)
add_subdirectory(MBB)
add_subdirectory(tool)

cet_cmake_config()
