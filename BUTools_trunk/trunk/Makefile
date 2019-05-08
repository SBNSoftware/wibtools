PACKAGES = \
	exception \
	WIB \
	tool \
	ups_package
# The packages appear to need to be built in the order above
# This does not prevent the contents of each package from building in parallel
.NOTPARALLEL:

ifndef WIB_PREFIX
$(error source environment script before building)
endif

VIRTUAL_PACKAGES = $(addsuffix /.virtual.Makefile,${PACKAGES})

FLAGS = $(ifeq $(MAKEFLAGS) "","",-$(MAKEFLAGS))

TARGETS=clean build all

.PHONY: $(TARGETS)
default: build

$(TARGETS): ${VIRTUAL_PACKAGES}

${VIRTUAL_PACKAGES}:
	${MAKE} ${FLAGS} -C $(@D) $(MAKECMDGOALS)

