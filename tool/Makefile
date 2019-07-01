PackagePath = $(shell pwd)

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
LIBRARY_WIBDEVICE_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_WIBDEVICE_SOURCES})
WIB_INCLUDE_PATH=${WIB_PREFIX}/include
WIB_LIBRARY_PATH=${WIB_PREFIX}/lib

LIBRARY_MBBDEVICE = lib/libWIBTool_MBBDevice.so
LIBRARY_MBBDEVICE_SOURCES = $(wildcard src/mbb_device/*.cc)
LIBRARY_MBBDEVICE_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_MBBDEVICE_SOURCES})
MBB_INCLUDE_PATH=${MBB_PREFIX}/include
MBB_LIBRARY_PATH=${MBB_PREFIX}/lib

EXECUTABLE_SOURCES = $(wildcard src/tool/*.cxx)
EXECUTABLE_OBJECT_FILES = $(patsubst src/%.cxx,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst src/%.cxx,bin/%.exe,${EXECUTABLE_SOURCES})

INCLUDE_PATH = \
							-Iinclude  \
							-I$(WIB_INCLUDE_PATH) \
							-I$(MBB_INCLUDE_PATH) \
			  				-I$(EXCEPTION_INCLUDE_PATH)

LIBRARY_PATH = \
							-Llib \
							-L$(WIB_LIBRARY_PATH) \
							-L$(MBB_LIBRARY_PATH) \
							-L${EXCEPTION_LIB_PATH}

ifdef BOOST_INC
INCLUDE_PATH +=-I$(BOOST_INC)
endif
ifdef BOOST_LIB
LIBRARY_PATH +=-L$(BOOST_LIB)
endif

LIBRARIES =    	-lToolException	\
		-lMBB \
		-ldl \
		-lreadline 			\
		-lcurses 			\
		-lboost_regex




EXECUTABLE_LINKED_LIBRARIES = ${LIBRARY_TOOL}  ${LIBRARY_WIBDEVICE}  ${LIBRARY_MBBDEVICE} ${LIBRARY_DUNETSDEVICE} ${LIBRARY_NGFECDEVICE}
EXECUTABLE_LINKED_LIBRARY_FLAGS = $(patsubst lib%,-l%,$(patsubst %.so,%,$(notdir ${EXECUTABLE_LINKED_LIBRARIES})))

EXECUTABLE_LIBRARIES = ${LIBRARIES} ${EXECUTABLE_LINKED_LIBRARY_FLAGS}


CPP_FLAGS = -std=c++17 -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Wimplicit-fallthrough=0 -Werror -Wno-literal-suffix -Wno-error=unused-variable -Wno-error=unused-function

## To compile more like artDAQ
#CPP_FLAGS +=-fno-omit-frame-pointer -pedantic -std=c++14 -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual
CPP_FLAGS +=-fno-omit-frame-pointer -Wno-ignored-qualifiers -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs  -Woverloaded-virtual

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

LINK_EXECUTABLE_FLAGS = -Xlinker "--no-as-needed" -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${EXECUTABLE_LIBRARIES} 


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
# WIBDevice library
# ------------------------
${LIBRARY_WIBDEVICE}: ${LIBRARY_WIBDEVICE_OBJECT_FILES} ${LIBRARY_HELPERS}
	mkdir -p $(dir $@)
	g++ ${LIBRARY_WIBDEVICE_OBJECT_FILES} -lWIB -lWIBTool_Helpers -o $@ ${LINK_LIBRARY_FLAGS}

${LIBRARY_WIBDEVICE_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	g++ ${CPP_FLAGS} -c $< -o $@

# ------------------------
# MBBDevice library
# ------------------------
${LIBRARY_MBBDEVICE}: ${LIBRARY_MBBDEVICE_OBJECT_FILES} ${LIBRARY_HELPERS}
	mkdir -p $(dir $@)
	g++ ${LIBRARY_MBBDEVICE_OBJECT_FILES} -lWIB -lWIBTool_Helpers -o $@ ${LINK_LIBRARY_FLAGS}

${LIBRARY_MBBDEVICE_OBJECT_FILES}: obj/%.o : src/%.cc 
	echo ${WIB_PREFIX}
	echo ${MBB_PREFIX}
	mkdir -p $(dir $@)
	g++ ${CPP_FLAGS} -c $< -o $@

-include $(LIBRARY_OBJECT_FILES:.o=.d)

