#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

def procSLTLLexicalUnit(fd): #{
	escaped = False; 	
	sl = '';
	tl = [];

	c = fd.read(1);
	while c != '/' and not escaped: #{
		sl = sl + c;
		c = fd.read(1);
	#}

	buf = '';
	c = fd.read(1);
	while c != '$': #{
		if c == '\\': 
			escaped = True; 
			buf = buf + c;
			c = fd.read(1);
			continue;

		if c == '/' and not escaped and buf: 
			tl.append(buf);
			buf = '';
			c = fd.read(1);
			continue;

		buf = buf + c;
		c = fd.read(1);
		escaped = False;
	#}
	tl.append(buf);

	if len(tl) > 1: #{
		return tl;
	else: #{
		return [];
	#}
#}

def usage(): #{
	print 'apertium-train-lex-freq.py <sl/tl tagged> <tl tagged>';
#}

if len(sys.argv) < 3: #{
	usage();
	sys.exit(-1);
#}

sltl_crp = file(sys.argv[1]);
tl_crp = file(sys.argv[2]);

analyses = {};

sys.stderr.write('Reading possible TL analyses from SL/TL corpus...\n');
escaped = False;
c = sltl_crp.read(1);
lu_count = 1;
while c: #{
	if c == '\\': 
		escaped = True;
		c = sltl_crp.read(1);
		continue;

	if c == '^' and not escaped: #{
		tl_parts = procSLTLLexicalUnit(sltl_crp);
		for part in tl_parts: #{
			if part not in analyses: #{
				analyses[part] = 0;
			#}	
		#}
		if lu_count % 100000 == 0: #{
			sys.stderr.write('.');
		#}	
		lu_count = lu_count + 1;
	elif c == '[' and not escaped: #{
		while c and c != ']':  #{
			c = sltl_crp.read(1);
		#}
	#}
		
	c = sltl_crp.read(1);
	escaped = False;
#}
sys.stderr.write('\nFound ' + str(len(analyses)) + ' analyses.\n');
sys.stderr.write('\nMaking analysis counts from TL tagged corpus...\n');

escaped = False;
lu_count = 1;
c = tl_crp.read(1);

while c: #{
	if c == '\\': 
		escaped = True;
		c = tl_crp.read(1);
		continue;

	if c == '^' and not escaped: #{
		sf = '';
		while c != '/' and not escaped: #{
			sf = sf + c;
			c = tl_crp.read(1);
		#}
		c = tl_crp.read(1);
		an = '';
		while c != '$' and not escaped: #{
			an = an + c;
			c = tl_crp.read(1);	
		#}
		if an in analyses: #{
			analyses[an] = analyses[an] + 1;
		#}
		
		if lu_count % 100000 == 0: #{
			sys.stderr.write('.');
		#}	
		if lu_count % 1000000 == 0: #{
			sys.stderr.write('.' + str(lu_count) + '\n');
		#}	
		lu_count = lu_count + 1;

	elif c == '[' and not escaped: #{
		while c and c != ']':  #{
			c = tl_crp.read(1);
		#}
	#}
		
	c = tl_crp.read(1);
	escaped = False;
#}


for analysis in analyses:
	print analyses[analysis] , '\t' , analysis;

