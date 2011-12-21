PREFIX=/home/fran/local
CFLAGS=-lxml2 -llttoolbox3  -L$(PREFIX)/lib -I$(PREFIX)/include/lttoolbox-3.2 -I. -I/usr/include/libxml2

all:
	g++ -ggdb -Wall -c -o lrx_compiler.o lrx_compiler.cc $(CFLAGS)
	g++ -ggdb -Wall -c -o lrx_processor.o lrx_processor.cc $(CFLAGS)
	g++ -ggdb -Wall -o apertium-lrx-comp lrx_comp.cc lrx_compiler.o $(CFLAGS)
	g++ -ggdb -Wall -o apertium-lrx-proc lrx_proc.cc lrx_processor.o $(CFLAGS)
	g++ -ggdb -Wall -o apertium-ldx-proc ldx_proc.cc $(CFLAGS)
