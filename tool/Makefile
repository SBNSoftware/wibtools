PackagePath = $(shell pwd)

ifdef CACTUS_ROOT
include $(PackagePath)/config/mfCommonDefs.mk
endif

#Exception class
EXCEPTION_INCLUDE_PATH=${EXCEPTION_BASE_PREFIX}/include/
EXCEPTION_LIB_PATH=${EXCEPTION_BASE_PREFIX}/lib


LIBRARY_TOOL = lib/libWIBTool.so
LIBRARY_TOOL_SOURCES  = $(wildcard src/tool/*.cc)
LIBRARY_TOOL_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_TOOL_SOURCES})

LIBRARY_HELPERS = lib/libWIBTool_Helpers.so
LIBRARY_HELPERS_SOURCES  = $(wildcard src/helpers/*.cc)
LIBRARY_HELPERS_SOURCES += $(wildcard src/helpers/StatusDisplay/*.cc)
LIBRARY_HELPERS_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_HELPERS_SOURCES})

LIBRARY_WIBDEVICE = lib/libWIBTool_WIBDevice.so
LIBRARY_WIBDEVICE_SOURCES = $(wildcard src/wib_device/*.cc)
#LIBRARY_WIBDEVICE_SOURCES += $(wildcard src/helpers/StatusDisplay*.cc)
LIBRARY_WIBDEVICE_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_WIBDEVICE_SOURCES})
WIB_INCLUDE_PATH=${WIB_PREFIX}/include/
WIB_LIBRARY_PATH=${WIB_PREFIX}/lib

ifdef CACTUS_ROOT
# UHal devices
LIBRARY_DUNETSDEVICE = lib/libcactus_WIBTool_DUNETSDevice.so
LIBRARY_DUNETSDEVICE_SOURCES  = $(wildcard src/dunets_device/*.cc)
LIBRARY_DUNETSDEVICE_SOURCES += $(wildcard src/uhal_helpers/*.cc)
LIBRARY_DUNETSDEVICE_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_DUNETSDEVICE_SOURCES})

LIBRARY_NGFECDEVICE = lib/libcactus_WIBTool_NGFECDevice.so
LIBRARY_NGFECDEVICE_SOURCES  = $(wildcard src/ngfec_device/*.cc)
LIBRARY_NGFECDEVICE_SOURCES += $(wildcard src/uhal_helpers/*.cc)
LIBRARY_NGFECDEVICE_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_NGFECDEVICE_SOURCES})
endif

EXECUTABLE_SOURCES = $(wildcard src/tool/*.cxx)
EXECUTABLE_OBJECT_FILES = $(patsubst src/%.cxx,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst src/%.cxx,bin/%.exe,${EXECUTABLE_SOURCES})

INCLUDE_PATH = \
							-Iinclude  \
							-I$(WIB_INCLUDE_PATH) \
			  				-I$(EXCEPTION_INCLUDE_PATH)

LIBRARY_PATH = \
							-Llib \
							-L$(WIB_LIBRARY_PATH) \
							-L${EXCEPTION_LIB_PATH}

ifdef BOOST_INC
INCLUDE_PATH +=-I$(BOOST_INC)
endif
ifdef BOOST_LIB
LIBRARY_PATH +=-L$(BOOST_LIB)
endif

LIBRARIES =    	-lToolException	\
		-ldl \
		-lreadline 			\
		-lcurses 			\
		-lboost_regex




EXECUTABLE_LINKED_LIBRARIES = ${LIBRARY_TOOL}  ${LIBRARY_WIBDEVICE}  ${LIBRARY_DUNETSDEVICE} ${LIBRARY_NGFECDEVICE}
EXECUTABLE_LINKED_LIBRARY_FLAGS = $(patsubst lib%,-l%,$(patsubst %.so,%,$(notdir ${EXECUTABLE_LINKED_LIBRARIES})))

EXECUTABLE_LIBRARIES = ${LIBRARIES} ${EXECUTABLE_LINKED_LIBRARY_FLAGS}


CPP_FLAGS = -std=c++11 -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Werror -Wno-literal-suffix -Wno-error=unused-variable -Wno-error=unused-function

## To compile more like artDAQ
#CPP_FLAGS +=-fno-omit-frame-pointer -pedantic -std=c++14 -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual
CPP_FLAGS +=-fno-omit-frame-pointer -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

LINK_EXECUTABLE_FLAGS = -Xlinker "--no-as-needed" -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${EXECUTABLE_LIBRARIES} 


# ------------------------
# IPBUS stuff
# ------------------------
ifdef CACTUS_ROOT
UHAL_LIBRARIES = -lcactus_uhal_log 		\
                 -lcactus_uhal_grammars 	\
                 -lcactus_uhal_uhal 		

UHAL_INCLUDE_PATH = \
	         					-isystem$(CACTUS_ROOT)/include 

UHAL_LIBRARY_PATH = \
							-L$(CACTUS_ROOT)/lib 

UHAL_CPP_FLAGHS = ${UHAL_INCLUDE_PATH}

UHAL_LIBRARY_FLAGS = ${UHAL_LIBRARY_PATH}

EXECUTABLE_LINKED_LIBRARIES += ${LIBRARY_DUNETSDEVICE} ${LIBRARY_NGFECDEVICE}
endif




.PHONY: all _all clean _cleanall build _buildall

default: build
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf bin
	rm -rf lib


all: _all
build: _all
buildall: _all
_all: ${EXECUTABLE_LINKED_LIBRARIES} ${LIBRARY_HELPERS} ${LIBRARY_TOOL}  ${EXECUTABLES}

# ------------------------
# Executables
# ------------------------
${EXECUTABLES}: bin/%.exe: obj/%.o ${EXECUTABLE_OBJECT_FILES}  ${LIBRARY_TOOL}  ${EXECUTABLE_LINKED_LIBRARIES}
	mkdir -p $(dir $@)
	g++ $< -o $@ ${LINK_EXECUTABLE_FLAGS}

${EXECUTABLE_OBJECT_FILES}: obj/%.o : src/%.cxx
	mkdir -p $(dir $@)
	g++ -c ${CPP_FLAGS}  $< -o $@

-include $(EXECUTABLE_OBJECT_FILES:.o=.d)

# ------------------------
# tool library
# ------------------------
${LIBRARY_TOOL}: ${LIBRARY_TOOL_OBJECT_FILES}
	mkdir -p $(dir $@)
	g++ ${LIBRARY_TOOL_OBJECT_FILES} -o $@ ${LINK_LIBRARY_FLAGS}

${LIBRARY_TOOL_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	g++ ${CPP_FLAGS} -c $< -o $@

# ------------------------
# helpers library
# ------------------------
${LIBRARY_HELPERS}: ${LIBRARY_HELPERS_OBJECT_FILES}
	mkdir -p $(dir $@)
	g++ ${LIBRARY_HELPERS_OBJECT_FILES} -o $@ ${LINK_LIBRARY_FLAGS}

${LIBRARY_HELPERS_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	g++ ${CPP_FLAGS} ${UHAL_CPP_FLAGHS} -c $< -o $@

# ------------------------
# WIBDEVICE library
# ------------------------
${LIBRARY_WIBDEVICE}: ${LIBRARY_WIBDEVICE_OBJECT_FILES} ${LIBRARY_HELPERS}
	mkdir -p $(dir $@)
	g++ ${LIBRARY_WIBDEVICE_OBJECT_FILES} -lWIB -lWIBTool_Helpers -o $@ ${LINK_LIBRARY_FLAGS}

${LIBRARY_WIBDEVICE_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	g++ ${CPP_FLAGS} -c $< -o $@

# ------------------------
# DUNETSDEVICE library
# ------------------------
ifdef CACTUS_ROOT
${LIBRARY_DUNETSDEVICE}: ${LIBRARY_DUNETSDEVICE_OBJECT_FILES} ${LIBRARY_HELPERS}
	g++ -o $@ ${LIBRARY_DUNETSDEVICE_OBJECT_FILES} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} -lWIBTool_Helpers

${LIBRARY_DUNETSDEVICE_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	mkdir -p {lib,obj}
	g++ ${CPP_FLAGS} ${UHAL_CPP_FLAGHS} -c $< -o $@
endif

# ------------------------
# NGFECDEVICE library
# ------------------------
ifdef CACTUS_ROOT
${LIBRARY_NGFECDEVICE}: ${LIBRARY_NGFECDEVICE_OBJECT_FILES} ${LIBRARY_HELPERS}
	g++ -o $@ ${LIBRARY_NGFECDEVICE_OBJECT_FILES} ${LINK_LIBRARY_FLAGS} ${UHAL_LIBRARY_FLAGS} ${UHAL_LIBRARIES} -lWIBTool_Helpers

${LIBRARY_NGFECDEVICE_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	mkdir -p {lib,obj}
	g++ ${CPP_FLAGS} ${UHAL_CPP_FLAGHS} -c $< -o $@
endif


-include $(LIBRARY_OBJECT_FILES:.o=.d)

