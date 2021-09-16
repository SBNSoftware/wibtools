#[================================================================[.rst:
X
=
#]================================================================]
########################################################################
# install_other()
#
#   Install other files under ${${CETMODULES_CURRENT_PROJECT_NAME}_INSTALLED_OTHER_DIR}
#
# Usage: install_other([SUBDIRNAME <subdir>] LIST ...)
#        install_other([SUBDIRNAME <subdir>] GLOB ...)
#        install_other([SUBDIRNAME <subdir>] [BASENAME_EXCLUDES ...]
#          [EXCLUDES ...] [EXTRAS ...] [SUBDIRS ...])
#
# See CetInstall.cmake for full usage description.
#
# Recognized all filenames.
#
# Excluded files:
#   ?*.bak ?*.~ ?*.~[0-9]* ?*.old ?*.orig ?*.rej #*# .DS_Store
#
########################################################################

# Avoid unwanted repeat inclusion.
include_guard(DIRECTORY)

cmake_policy(PUSH)
cmake_minimum_required(VERSION 3.18.2 FATAL_ERROR)

include(CetInstall)
include(CetPackagePath)
include(ProjectVariable)

function(install_other)
  list(REMOVE_ITEM ARGN PROGRAMS) # Not meaningful.
  cmake_parse_arguments(PARSE_ARGV 0 IS "" "SUBDIRNAME" "")

  if (NOT "INSTALLED_OTHER_${IS_SUBDIRNAME}_DIR" IN_LIST CETMODULES_VARS_PROJECT_${CETMODULES_CURRENT_PROJECT_NAME})
    project_variable(INSTALLED_OTHER_${IS_SUBDIRNAME}_DIR ${IS_SUBDIRNAME} CONFIG
      OMIT_IF_EMPTY OMIT_IF_MISSING OMIT_IF_NULL
      DOCSTRING "Directory below prefix to install other files for debug and other purposes")
  endif()

  if ("LIST" IN_LIST IS_UNPARSED_ARGUMENTS)
    _cet_install(other ${CETMODULES_CURRENT_PROJECT_NAME}_INSTALLED_OTHER_${IS_SUBDIRNAME}_DIR ${IS_UNPARSED_ARGUMENTS}
      SUBDIRNAME "./" _INSTALL_ONLY)
  elseif("GLOB" IN_LIST IS_UNPARSED_ARGUMENTS)
    cmake_parse_arguments(PARSE_ARGV 0 IS "" "" "GLOB")
    file(GLOB FILE_LIST ${IS_GLOB})
    list(APPEND IS_UNPARSED_ARGUMENTS LIST ${FILE_LIST})
    _cet_install(other ${CETMODULES_CURRENT_PROJECT_NAME}_INSTALLED_OTHER_${IS_SUBDIRNAME}_DIR ${IS_UNPARSED_ARGUMENTS}
      SUBDIRNAME "./" _INSTALL_ONLY)
  else()
    _cet_install(other ${CETMODULES_CURRENT_PROJECT_NAME}_INSTALLED_OTHER_${IS_SUBDIRNAME}_DIR ${IS_UNPARSED_ARGUMENTS}
      SUBDIRNAME "./"
      _SEARCH_BUILD _INSTALL_ONLY
      _EXTRA_BASENAME_EXCLUDES "?*.bak" "?*.~" "?*.~[0-9]*" "?*.old" "?*.orig" "?*.rej" "#*#" ".DS_Store" "CMakeLists.txt" "?*.cmake" "Makefile"
      _GLOBS "?*")
  endif()
endfunction()

cmake_policy(POP)
