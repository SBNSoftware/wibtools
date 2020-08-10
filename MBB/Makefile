LDFLAGS+=-shared -fPIC -O3 -Wall -Werror -rdynamic -Llib -g 
CXXFLAGS+=-g -Wall -O3 -Werror -fPIC

## To compile more like artDAQ
#CXXFLAGS +=-g -gdwarf-4 -O3 -fno-omit-frame-pointer -std=c++98 -Werror -pedantic -DRTI_UNIX -DRTI_LINUX -DRTI_64BIT -m64 -std=c++14 -Wall -Werror=return-type -Wextra -Wno-long-long -Winit-self -Wno-unused-local-typedefs -Woverloaded-virtual -Wnon-virtual-dtor -Wdelete-non-virtual-dtor

ifdef BOOST_LIB
LDFLAGS +=-L${BOOST_LIB}
endif
ifdef BOOST_INC
CXXFLAGS +=-I${BOOST_INC}
endif

ifdef TRACE_INC
CXXFLAGS +=-I$(TRACE_INC)
endif

BUILD_PATH=./build
SRC_PATH=./src
INCLUDE_PATH=./include
LIB_PATH=./lib

#MBB CLass
MBB_LIB=${LIB_PATH}/libMBB.so
MBB_INCLUDE_PATH=${INCLUDE_PATH}
MBB_SRC_PATH=${SRC_PATH}
MBB_BUILD_PATH=${BUILD_PATH}
CXXFLAGS +=-I${MBB_INCLUDE_PATH} -I${WIB_PREFIX}/include
LDFLAGS +=-lboost_regex
MBB_SRC=$(notdir $(wildcard ${MBB_SRC_PATH}/*.cc))
MBB_OBJ=$(patsubst %.cc,${MBB_BUILD_PATH}/%.o,${MBB_SRC})

#Exception class
EXCEPTION_INCLUDE_PATH=${EXCEPTION_BASE_PREFIX}/include/
EXCEPTION_LIB_PATH=${EXCEPTION_BASE_PREFIX}/lib
CXXFLAGS +=-I${EXCEPTION_INCLUDE_PATH}
#LDFLAGS +=-L${EXCEPTION_LIB_PATH} 
#LDFLAGS +=-lToolException

all: ${MBB_LIB}

debug :
	@echo ${MBB_OBJ}

${MBB_LIB}	: ${MBB_OBJ}
	mkdir -p $(@D)
	${CXX} -o $@ $^ ${LDFLAGS}


clean : 
	@rm -rf ${LIB_PATH}
	@rm -rf ${BUILD_PATH}

${BUILD_PATH}/%.o : ${SRC_PATH}/%.cc
	mkdir -p ${BUILD_PATH}
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -c $< -o $@

