h_sources = irstlm_ranker.h lrx_compiler.h lrx_processor.h multi_translator.h \
			tagger_output_processor.h weight.h
cc_sources = lrx_compiler.cc lrx_processor.cc multi_translator.cc \
			 tagger_output_processor.cc

library_includedir = $(includedir)/$(PACKAGE_NAME)-$(VERSION_API)/$(PACKAGE_NAME)
library_include_HEADERS = $(h_sources)
lib_LTLIBRARIES = libapertium-lex-tools0.la
libapertium_lex_tools0_la_SOURCES = $(h_sources) $(cc_sources)
libapertium_lex_tools0_la_LDFLAGS = -version-info $(SOVERSION) -release $(VERSION_API)

bin_PROGRAMS = lrx-comp lrx-proc multitrans process-tagger-output

lrx_comp_SOURCES = lrx_comp.cc

lrx_proc_SOURCES = lrx_proc.cc

multitrans_SOURCES = multitrans.cc

process_tagger_output_SOURCES = process_tagger_output.cc

LDADD = -lapertium-lex-tools$(VERSION_MAJOR) $(lib_LTLIBRARIES)

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
