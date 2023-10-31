AM_LDFLAGS=$(LIBS)

SUBDIRS = src scripts locales

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

export ALT_I18N_DATA=$(datadir)/lttoolbox/lttoolbox.dat
export LOCALES_DIR=$(datadir)/$(PACKAGE_NAME)
export ALX_I18N_DATA=$(LOCALES_DIR)/alx.dat
