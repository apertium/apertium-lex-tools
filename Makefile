CFLAGS=-llttoolbox3  -L/home/fran/local/lib -I/home/fran/local/include/lttoolbox-3.2

all:
	g++ -Wall apertium_lex_rule_comp.cc -o apertium-lex-rule-comp $(CFLAGS)
	g++ -Wall apertium_lex_rule_proc.cc -o apertium-lex-rule-proc $(CFLAGS)
	g++ -Wall apertium_lex_defaults.cc -o apertium-lex-defaults $(CFLAGS)
