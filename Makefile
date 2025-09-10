# PQL Project Makefile
# Linux-specific build configuration

# Architecture and build settings for Linux
ARCH = LINUX
DEBUG = -g -Wall
PQLCFLAGS = $(shell pkg-config --cflags gtk+-2.0 glib-2.0) -I/usr/include/tirpc
PQLLFLAGS = -lm $(shell pkg-config --libs gtk+-2.0 glib-2.0) -ltirpc -lnsl

# Project directories
PQLHOME = $(CURDIR)
PQLLIB = $(PQLHOME)/lib/$(ARCH)
PQLINC = $(PQLHOME)/include
PQLBIN = $(PQLHOME)/bin/$(ARCH)

# Export variables for sub-makes
export ARCH PQLHOME PQLLIB PQLINC PQLBIN DEBUG PQLCFLAGS PQLLFLAGS

# Contrib libraries
CONTRIB_LIBS = libvfbb.a libmseed.a libgdbm.a libtdb.a
CONTRIB_TARGETS = contrib-vfbb contrib-libmseed contrib-gdbm contrib-tdb

# Utility libraries
UTIL_LIBS = libdata.a libutils.a libtform.a libdbl.a
UTIL_TARGETS = utils-data utils-hlpr utils-tform utils-gdbm utils-dbl

# All libraries
ALL_LIBS = $(addprefix $(PQLLIB)/, $(CONTRIB_LIBS) $(UTIL_LIBS))

# Main target
.PHONY: all clean install contrib utils main verify
.DEFAULT_GOAL := all

all: setup contrib utils main verify

# Create necessary directories
setup:
	@mkdir -p $(PQLBIN) $(PQLLIB)
	@mkdir -p src/LINUX
	@mkdir -p utils/data/LINUX utils/hlpr/LINUX utils/tform/LINUX
	@mkdir -p utils/gdbm/src/LINUX utils/dbl/src/LINUX
	@mkdir -p contrib/vfbb/LINUX

# Contrib library targets
contrib: $(CONTRIB_TARGETS)

contrib-vfbb:
	@echo "Building VFBB library..."
	cd contrib/vfbb/LINUX && $(MAKE)
	cp contrib/vfbb/LINUX/libvfbb.a $(PQLLIB)/
	ranlib $(PQLLIB)/libvfbb.a

contrib-libmseed:
	@echo "Building libmseed library..."
	cd contrib/libmseed && $(MAKE)
	cp contrib/libmseed/libmseed.a $(PQLLIB)/
	ranlib $(PQLLIB)/libmseed.a

contrib-gdbm:
	@echo "Building GDBM library..."
	cd contrib/gdbm && \
	if [ ! -e $(PQLLIB)/libgdbm.a ]; then \
		CFLAGS="$(PQLCFLAGS)" \
		LDFLAGS="$(PQLLFLAGS)" \
		./configure; \
	fi && \
	$(MAKE)
	cp contrib/gdbm/.libs/libgdbm.a $(PQLLIB)/
	ranlib $(PQLLIB)/libgdbm.a

contrib-tdb:
	@echo "Building TDB library..."
	cd contrib/tdb && \
	if [ ! -e $(PQLLIB)/libtdb.a ]; then \
		$(MAKE) distclean; \
		./configure; \
	fi && \
	$(MAKE)
	ar cr $(PQLLIB)/libtdb.a contrib/tdb/bin/default/common/*.o
	ranlib $(PQLLIB)/libtdb.a

# Utility library targets
utils: $(UTIL_TARGETS)

utils-data:
	@echo "Building data utilities..."
	cd utils/data && $(MAKE)

utils-hlpr:
	@echo "Building helper utilities..."
	cd utils/hlpr && $(MAKE)

utils-tform:
	@echo "Building transform utilities..."
	cd utils/tform && $(MAKE)

utils-gdbm:
	@echo "Building GDBM utilities..."
	cd utils/gdbm/src && $(MAKE)

utils-dbl:
	@echo "Building DBL utilities..."
	cd utils/dbl/src && $(MAKE)

# Main executable
main: $(ALL_LIBS)
	@echo "Building main PQL executable..."
	cd src && $(MAKE)

# Verification target
verify:
	@echo
	@echo "Verifying PQL compilation:"
	@echo
	@cd $(PQLBIN) && \
	failed=false; \
	for exe in pql; do \
		if [ ! -e $$exe ]; then \
			echo "Missing executable file '$$exe'."; \
			failed=true; \
		fi; \
	done; \
	if [ "$$failed" = "false" ]; then \
		echo "PQL exe's Compiled and Installed Successfully..."; \
	else \
		echo "PQL Verification FAILED!"; \
		cd $(PQLLIB) && \
		for lib in $(CONTRIB_LIBS) $(UTIL_LIBS); do \
			if [ ! -e $$lib ]; then \
				echo "Required PQL Library '$$lib' does not exist."; \
				echo "	Compilation most likely failed, please investigate..."; \
			fi; \
		done; \
		exit 1; \
	fi
	@echo

# Clean targets
clean: clean-contrib clean-utils clean-main clean-build

clean-contrib:
	@echo "Cleaning contrib libraries..."
	-cd contrib/vfbb/LINUX && $(MAKE) clean
	-cd contrib/libmseed && $(MAKE) clean
	-cd contrib/gdbm && $(MAKE) clean
	-cd contrib/tdb && $(MAKE) clean

clean-utils:
	@echo "Cleaning utility libraries..."
	-cd utils/data && $(MAKE) clean
	-cd utils/hlpr && $(MAKE) clean
	-cd utils/tform && $(MAKE) clean
	-cd utils/gdbm/src && $(MAKE) clean
	-cd utils/dbl/src && $(MAKE) clean

clean-main:
	@echo "Cleaning main executable..."
	-cd src && $(MAKE) clean

clean-build:
	@echo "Cleaning build artifacts..."
	-rm -f $(PQLLIB)/*.a
	-find . -name '*~' -delete

# Install target (optional)
install: all
	@echo "Installing PQL..."
	@echo "Note: Modify this target to specify installation directories"
	@echo "Currently PQL is built in: $(PQLBIN)"

# Help target
help:
	@echo "PQL Makefile Help"
	@echo "================="
	@echo "Targets:"
	@echo "  all          - Build everything (default)"
	@echo "  contrib      - Build only contrib libraries"
	@echo "  utils        - Build only utility libraries"
	@echo "  main         - Build only main executable"
	@echo "  clean        - Clean all build artifacts"
	@echo "  clean-contrib- Clean only contrib libraries"
	@echo "  clean-utils  - Clean only utility libraries"
	@echo "  clean-main   - Clean only main executable"
	@echo "  verify       - Verify build completion"
	@echo "  install      - Install PQL (customize as needed)"
	@echo "  help         - Show this help"
	@echo
	@echo "Architecture detected: $(ARCH)"
	@echo "Build directories:"
	@echo "  Binaries: $(PQLBIN)"
	@echo "  Libraries: $(PQLLIB)"
	@echo "  Headers: $(PQLINC)"
