bin_PROGRAMS = lrx-comp lrx-proc multitrans process-tagger-output

lrx_comp_SOURCES = lrx_compiler.cc lrx_comp.cc

lrx_proc_SOURCES = lrx_processor.cc lrx_proc.cc

multitrans_SOURCES = tagger_output_processor.cc multi_translator.cc multitrans.cc

process_tagger_output_SOURCES = process_tagger_output.cc

if HAVE_YASMET
    yasmet_SOURCES = yasmet.cc
    bin_PROGRAMS += yasmet
endif

if HAVE_IRSTLM
    irstlm_ranker_SOURCES = irstlm_ranker.cc
    irstlm_ranker_LDADD = -lirstlm
    bin_PROGRAMS += irstlm-ranker
endif

EXTRA_DIST = lrx_compiler.h lrx_processor.h multi_translator.h tagger_output_processor.h
