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

BUILD_PATH=./build
SRC_PATH=./src
INCLUDE_PATH=./include
LIB_PATH=./lib

#WIB CLass
WIB_LIB=${LIB_PATH}/libWIB.so
WIB_INCLUDE_PATH=${INCLUDE_PATH}
WIB_SRC_PATH=${SRC_PATH}
WIB_BUILD_PATH=${BUILD_PATH}
CXXFLAGS +=-I${WIB_INCLUDE_PATH}
LDFLAGS +=-lboost_regex
WIB_SRC=$(notdir $(wildcard ${WIB_SRC_PATH}/*.cc))
WIB_OBJ=$(patsubst %.cc,${WIB_BUILD_PATH}/%.o,${WIB_SRC})
VERSION=${WIB_BUILD_PATH}/WIBBase_Version
VERSION_SRC=${VERSION}.cc
VERSION_OBJ=${VERSION}.o

#Exception class
EXCEPTION_INCLUDE_PATH=${EXCEPTION_BASE_PREFIX}/include/
EXCEPTION_LIB_PATH=${EXCEPTION_BASE_PREFIX}/lib
CXXFLAGS +=-I${EXCEPTION_INCLUDE_PATH}
#LDFLAGS +=-L${EXCEPTION_LIB_PATH} 
#LDFLAGS +=-lToolException

all: ${WIB_LIB}

debug :
	@echo ${WIB_OBJ}

${WIB_LIB}	: ${WIB_OBJ}
	mkdir -p $(@D)
	@#Build version source file for the WIBBase class
	@echo ${VERSION_SRC}
	@mkdir -p ${BUILD_PATH}
	@echo "#include <WIBBase.hh>" > ${VERSION_SRC}
	@svn -R info | awk 'BEGIN{version=0}{if($$3 == "Rev:"){if($$4 > version){version = $$4}}}END{print "const int WIBBase::Version(" version ");"}' >> ${VERSION_SRC}
	${CXX} ${CXXFLAGS} -c ${VERSION_SRC} -o ${VERSION_OBJ} ; rm ${VERSION_SRC} 	
	${CXX} -o $@ $^ ${VERSION_OBJ} ${LDFLAGS}
	@rm ${VERSION_OBJ}

clean : 
	@rm -rf ${LIB_PATH}
	@rm -rf ${BUILD_PATH}

${BUILD_PATH}/%.o : ${SRC_PATH}/%.cc
	mkdir -p ${BUILD_PATH}
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -c $< -o $@

