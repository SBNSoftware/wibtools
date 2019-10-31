
ifdef QUALIFIER

QUAL := $(subst :,_,${QUALIFIER})
FLAVOR := $(shell ups flavor -4)
FLAVOR := $(strip ${FLAVOR})
#VERSION := $(shell svn info | sed -n 's/Revision: \([0-9]*\)/\1/p')
VERSION := $(strip ${VERSION})

MAINDIR := $(strip wibtools/v${VERSION})
QUALDIR := $(strip ${MAINDIR}/${FLAVOR}-${QUAL})
VERSIONDIR := $(strip wibtools/v${VERSION}.version)
VERSIONFILE := $(strip ${VERSIONDIR}/${FLAVOR}-${QUAL})

$(info FLAVOR is ${FLAVOR})
$(info QUAL is ${QUAL})
$(info VERSION is ${VERSION})
$(info MAINDIR is ${MAINDIR})
$(info QUALDIR is ${QUALDIR})
$(info WIB_PREFIX is ${WIB_PREFIX})
$(info VERSIONDIR is ${VERSIONDIR})
$(info VERSIONFILE is ${VERSIONFILE})

TOOL_PREFIX = $(realpath ${WIB_PREFIX}/../tool)
EXCEPTION_PREFIX = $(realpath ${WIB_PREFIX}/../exception)

$(info TOOL_PREFIX ${TOOL_PREFIX})
$(info EXCEPTION_PREFIX_PREFIX ${EXCEPTION_PREFIX})

EXECUTABLES = $(wildcard ${TOOL_PREFIX}/bin/tool/*)
EXECUTABLES_FNS = $(notdir ${EXECUTABLES})
EXECUTABLES_DEST = $(addprefix ${QUALDIR}/bin/,${EXECUTABLES_FNS})

$(info EXECUTABLES is ${EXECUTABLES})
$(info EXECUTABLES_FNS is ${EXECUTABLES_FNS})
$(info EXECUTABLES_DEST is ${EXECUTABLES_DEST})

LIBS = $(wildcard ${WIB_PREFIX}/lib/*.so) $(wildcard ${MBB_PREFIX}/lib/*.so) $(wildcard ${TOOL_PREFIX}/lib/*.so) $(wildcard ${EXCEPTION_PREFIX}/lib/*.so)
LIBS_FNS = $(notdir ${LIBS})
LIBS_DEST = $(addprefix ${QUALDIR}/lib/,${LIBS_FNS})

$(info LIBS is ${LIBS})
$(info LIBS_FNS is ${LIBS_FNS})
$(info LIBS_DEST is ${LIBS_DEST})

WIB_INCLUDES = $(wildcard ${WIB_PREFIX}/include/*)
WIB_INCLUDES_FNS = $(notdir ${WIB_INCLUDES})
WIB_INCLUDES_DEST = $(addprefix ${MAINDIR}/include/,${WIB_INCLUDES_FNS})

MBB_INCLUDES = $(wildcard ${MBB_PREFIX}/include/*)
MBB_INCLUDES_FNS = $(notdir ${MBB_INCLUDES})
MBB_INCLUDES_DEST = $(addprefix ${MAINDIR}/include/,${MBB_INCLUDES_FNS})

EXCEPTION_INCLUDES = $(wildcard ${EXCEPTION_PREFIX}/include/WIBException/*)
EXCEPTION_INCLUDES_FNS = $(notdir ${EXCEPTION_INCLUDES})
EXCEPTION_INCLUDES_DEST = $(addprefix ${MAINDIR}/include/WIBException/,${EXCEPTION_INCLUDES_FNS})

$(info WIB_INCLUDES is ${WIB_INCLUDES})
$(info WIB_INCLUDES_DEST is ${WIB_INCLUDES_DEST})

$(info EXCEPTION_INCLUDES is ${EXCEPTION_INCLUDES})
$(info EXCEPTION_INCLUDES_DEST is ${EXCEPTION_INCLUDES_DEST})

CONFIGS = $(wildcard ${WIB_PREFIX}/config/*) $(wildcard ${TOOL_PREFIX}/config/*) $(wildcard ${EXCEPTION_PREFIX}/config/*)
CONFIGS_FNS = $(notdir ${CONFIGS})
CONFIGS_DEST = $(addprefix ${MAINDIR}/config/,${CONFIGS_FNS})

#$(info CONFIGS is ${CONFIGS})
#$(info CONFIGS_FNS is ${CONFIGS_FNS})
#$(info CONFIGS_DEST is ${CONFIGS_DEST})

TABLES = $(wildcard ${WIB_PREFIX}/tables/*)
TABLES_FNS = $(notdir ${TABLES})
TABLES_DEST = $(addprefix ${MAINDIR}/tables/,${TABLES_FNS})

#$(info TABLES is ${TABLES})
#$(info TABLES_FNS is ${TABLES_FNS})
#$(info TABLES_DEST is ${TABLES_DEST})

.PHONY: all _all clean _cleanall build _buildall

default: build
clean: _cleanall
_cleanall:
	#rm -rf ${MAINDIR}
	$(info Not cleaning UPS package)

all: _all
build: _all
buildall: _all
_all: ${EXECUTABLES_DEST} ${LIBS_DEST} ${WIB_INCLUDES_DEST} ${MBB_INCLUDES_DEST} ${EXCEPTION_INCLUDES_DEST} ${TABLES_DEST} ${CONFIGS_DEST} ${MAINDIR}/ups/wibtools.table ${VERSIONFILE}

# ------------------------
# Directory
# ------------------------

${QUALDIR} ${MAINDIR}:
	mkdir -p ${QUALDIR}

${MAINDIR}/ups/wibtools.table: ${MAINDIR}
	mkdir -p ${MAINDIR}/ups
	cp ups/wibtools.table ${MAINDIR}/ups/.

${VERSIONFILE}: ${MAINDIR}
	mkdir -p ${VERSIONDIR}
	cat ups/version_file_template.txt | sed 's/xVERSIONx/v${VERSION}/g' | sed 's/xFLAVORx/${FLAVOR}/g' | sed 's/xQUALIFIERx/${QUALIFIER}/g' | sed 's/xUSERx/${USER}/g' | sed "s/xDATEx/`date -u '+%Y-%m-%d %H.%M.%S GMT'`/g"> ${VERSIONFILE}

# ------------------------
# Executables
# ------------------------
${EXECUTABLES_DEST}: ${EXECUTABLES} ${QUALDIR}
	mkdir -p ${QUALDIR}/bin/
	cp $< $@

# ------------------------
# Libraries
# ------------------------
${LIBS_DEST}: ${LIBS} ${QUALDIR}
	mkdir -p ${QUALDIR}/lib/
	cp $(filter %$(subst ${QUALDIR},,${@}),${LIBS}) $@

# ------------------------
# Headers / Includes
# ------------------------
${WIB_INCLUDES_DEST}: ${WIB_INCLUDES} ${MAINDIR}
	mkdir -p ${MAINDIR}/include
	cp $(filter %$(subst ${MAINDIR},,${@}),${WIB_INCLUDES}) $@

${MBB_INCLUDES_DEST}: ${MBB_INCLUDES} ${MAINDIR}
	mkdir -p ${MAINDIR}/include
	cp $(filter %$(subst ${MAINDIR},,${@}),${MBB_INCLUDES}) $@

${EXCEPTION_INCLUDES_DEST}: ${EXCEPTION_INCLUDES}
	mkdir -p ${MAINDIR}/include/WIBException/
	cp $(filter %$(subst ${MAINDIR},,${@}),${EXCEPTION_INCLUDES}) $@

# ------------------------------
# Register maps and config files
# ------------------------------
${TABLES_DEST}: ${TABLES} ${MAINDIR}
	mkdir -p ${MAINDIR}/tables/
	cp -P $(filter %$(subst ${MAINDIR},,${@}),${TABLES}) $@

${CONFIGS_DEST}: ${CONFIGS} ${MAINDIR}
	mkdir -p ${MAINDIR}/config/
	cp -P $(filter %$(subst ${MAINDIR},,${@}),${CONFIGS}) $@

else

.PHONY: all _all clean _cleanall build _buildall

default: build
clean: _cleanall
_cleanall: ;

all: _all
build: _all
buildall: _all
_all: ;
endif #ifdef QUALIFIER
