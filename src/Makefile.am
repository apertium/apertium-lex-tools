h_sources = irstlm_ranker.h lrx_compiler.h lrx_processor.h multi_translator.h \
			tagger_output_processor.h weight.h
cc_sources = lrx_compiler.cc lrx_processor.cc multi_translator.cc \
			 tagger_output_processor.cc

library_includedir = $(includedir)/$(PACKAGE_NAME)
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libapertium-lex-tools.la
libapertium_lex_tools_la_SOURCES = $(h_sources) $(cc_sources)
libapertium_lex_tools_la_LDFLAGS = -version-info $(VERSION_ABI)

bin_PROGRAMS = lrx-comp lrx-proc multitrans process-tagger-output

lrx_comp_SOURCES = lrx_comp.cc

lrx_proc_SOURCES = lrx_proc.cc

multitrans_SOURCES = multitrans.cc

process_tagger_output_SOURCES = process_tagger_output.cc

AM_CPPFLAGS = -DALX_I18N_DATA='"$(ALX_I18N_DATA)"' -DALT_I18N_DATA='"$(ALT_I18N_DATA)"'

LDADD = -lapertium-lex-tools $(lib_LTLIBRARIES)

if HAVE_YASMET
    yasmet_SOURCES = yasmet.cc
    bin_PROGRAMS += yasmet
endif

if HAVE_IRSTLM
    irstlm_ranker_SOURCES = irstlm_ranker.cc
    irstlm_ranker_LDADD = -lirstlm
    bin_PROGRAMS += irstlm-ranker
endif

apertium-validate-lrx: Makefile.am validate-lrx.sh
	@echo "Creating apertium-validate-lrx script"
	@cat validate-lrx.sh > $@
	@echo "xmllint --dtdvalid \"$(prefix)\"/share/apertium-lex-tools/lrx.dtd --noout \"\$$FILE1\"" >> $@
	@chmod a+x $@

GENERATEDSCRIPTS = apertium-validate-lrx
bin_SCRIPTS = $(GENERATEDSCRIPTS)
CLEANFILES = *~ $(GENERATEDSCRIPTS)

apertium_lex_toolsdir = $(prefix)/share/apertium-lex-tools
apertium_lex_tools_DATA = lrx.dtd

EXTRA_DIST = lrx_compiler.h lrx_processor.h multi_translator.h tagger_output_processor.h validate-lrx.sh
