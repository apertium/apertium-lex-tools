AM_LDFLAGS=$(LIBS)

SUBDIRS = src

pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = apertium-lex-tools.pc

if HAVE_PYTHON_BINDINGS
SUBDIRS += python
endif

.PHONY: check
check:
	testing/run

.PHONY: test
test: check
