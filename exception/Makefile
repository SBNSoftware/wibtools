PackagePath = $(shell pwd)

LIBRARY_EXCEPTION = lib/libToolException.so
LIBRARY_EXCEPTION_SOURCES  = $(wildcard src/BUException/*.cc)
LIBRARY_EXCEPTION_OBJECT_FILES = $(patsubst src/%.cc,obj/%.o,${LIBRARY_EXCEPTION_SOURCES})


INCLUDE_PATH = \
							 -Iinclude  \

LIBRARY_PATH = \
							 -Llib \

ifdef BOOST_INC
INCLUDE_PATH +=-I$(BOOST_INC)
endif
ifdef BOOST_LIB
LIBRARY_PATH +=-L$(BOOST_LIB)
endif

LIBRARIES = -lboost_regex


CPP_FLAGS = -g -O3 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} -Werror

## To compile more like artDAQ
#CXXFLAGS +=-g -gdwarf-4 -O3 -fno-omit-frame-pointer -std=c++98 -Werror -pedantic -DRTI_UNIX -DRTI_LINUX -DRTI_64BIT -m64 -std=c++14 -Wall -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs -Woverloaded-virtual -Wnon-virtual-dtor -Wdelete-non-virtual-dtor

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O3 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}


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
_all: ${LIBRARY_EXCEPTION}


# ------------------------
# tool library
# ------------------------
${LIBRARY_EXCEPTION}: ${LIBRARY_EXCEPTION_OBJECT_FILES}
	mkdir -p $(dir $@)
	g++ ${LINK_LIBRARY_FLAGS} ${LIBRARY_EXCEPTION_OBJECT_FILES} -o $@

${LIBRARY_EXCEPTION_OBJECT_FILES}: obj/%.o : src/%.cc 
	mkdir -p $(dir $@)
	g++ ${CPP_FLAGS} -c $< -o $@

-include $(LIBRARY_OBJECT_FILES:.o=.d)

