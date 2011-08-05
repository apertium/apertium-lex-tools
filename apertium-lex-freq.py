#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

global debug;
freq_table = {};

def loadFreqTableFromFile(f): #{
	for line in file(f).read().split('\n'): #{
		if line.count('\t') < 1: #{
			continue;
		#}
		row = line.split('\t');
		frequency = int(row[0]);
		analysis = row[1];
	
		freq_table[analysis] = frequency;
	#}
	return freq_table;
#}

def procLexicalUnit(c): #{
	global debug;
	escaped = False; 	
	sl = '';
	tl = [];

	c = sys.stdin.read(1);
	while c != '/' and not escaped: #{
		sl = sl + c;
		c = sys.stdin.read(1);
	#}

	buf = '';
	c = sys.stdin.read(1);
	while c != '$': #{
		if c == '\\': 
			escaped = True; 
			buf = buf + c;
			c = sys.stdin.read(1);
			continue;

		if c == '/' and not escaped and buf: 
			tl.append(buf);
			buf = '';
			c = sys.stdin.read(1);
			continue;

		buf = buf + c;
		c = sys.stdin.read(1);
		escaped = False;
	#}
	tl.append(buf);

	i = 0;
	tl_freq = [];
	
	for i in range(0, len(tl)): #{
		if tl[i] in freq_table: 
			tl_freq.append((freq_table[tl[i]], tl[i]));
	#}

	tl_freq.sort();
	tl_freq.reverse();

	sys.stdout.write('^');
	sys.stdout.write(sl); 
	sys.stdout.write('/');

		
	if len(tl_freq) > 1 and debug: #{
		sys.stdout.write(tl_freq[0][1] + '/');
		for i in range(1, len(tl_freq)): #{
			sys.stdout.write('=' + tl_freq[i][1]);
		
			if i < (len(tl_freq) - 1): #{
				sys.stdout.write('/');
			#}
		#}

	elif len(tl_freq) > 0: #{
		sys.stdout.write(tl_freq[0][1]);

	else: #{
		for i in range(0, len(tl)): #{
			sys.stdout.write(tl[i]);
		
			if i < (len(tl) - 1): #{
				sys.stdout.write('/');
			#}
		#}
	#}
	sys.stdout.write('$');
#}

def usage(): #{
	print 'apertium-lex-freq.py [-d] <freq file>';
#}

debug = False;

if len(sys.argv) < 2: #{
	usage();
	sys.exit(-1);
#}

if sys.argv[1] == '-d': #{
	debug = True;
	freq_table = loadFreqTableFromFile(sys.argv[2]);
else: #{
	freq_table = loadFreqTableFromFile(sys.argv[1]);
#}

escaped = False;

c = sys.stdin.read(1);

while c: #{
	if c == '\\': 
		escaped = True;
		sys.stdout.write(c);
		c = sys.stdin.read(1);
		continue;

	if c == '^' and not escaped: #{
		procLexicalUnit(c);
	elif c == '[' and not escaped: #{
		sys.stdout.write(c);
		while c and c != ']':  #{
			c = sys.stdin.read(1);
			sys.stdout.write(c);
		#}
	else: #{
		sys.stdout.write(c);
	#}
		
	c = sys.stdin.read(1);
	escaped = False;
#}
