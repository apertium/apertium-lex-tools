AM_LDFLAGS=$(LIBS)
#AM_CXXFLAGS=-Wall -g -ggdb -DDEBUG 

# bin_PROGRAMS = lrx-comp lrx-proc multitrans irstlm-ranker #apertium-ldx-proc
bin_PROGRAMS = lrx-comp lrx-proc multitrans # irstlm-ranker apertium-ldx-proc

lrx_comp_SOURCES = lrx_compiler.cc lrx_comp.cc

lrx_proc_SOURCES = lrx_processor.cc lrx_proc.cc

multitrans_SOURCES = tagger_output_processor.cc multi_translator.cc multitrans.cc

# g++ -Wall -Wextra -O3 -std=c++98 yasmet.cc -o yasmet
if HAVE_YASMET
    yasmet_SOURCES = yasmet.cc
    bin_PROGRAMS += yasmet
endif

if HAVE_IRSTLM
    irstlm_ranker_SOURCES = irstlm_ranker.cpp
    bin_PROGRAMS += irstlm_ranker
endif


