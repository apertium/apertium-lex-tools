CFLAGS=-llttoolbox3  -L/home/fran/local/lib -I/home/fran/local/include/lttoolbox-3.2 -I. -I/usr/include/libxml2

all:
	g++ -Wall -c -o lrx_compiler.o lrx_compiler.cc $(CFLAGS)
	g++ -Wall -o apertium-lrx-comp lrx_comp.cc lrx_compiler.o $(CFLAGS)
	g++ -Wall -o apertium-ldx-proc ldx_proc.cc $(CFLAGS)

	g++ -Wall apertium_lex_rule_proc.cc -o apertium-lex-rule-proc $(CFLAGS)

