#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;
from operator import mul

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

t = 0
lineno = 0

def process_biltrans_unit(lu): #{

	state = 0;
	sl = '';
	tl = [];
	for c in lu[1:-1]: #{ 
		#^worth<n><sg>/valor<n><m><sg>$ ^\$<mon>/\$<mon>$^20<num>/20<num>$^*m/*m$ 
		#print c , sl , tl;
		if c == '/': #{
			state = state + 1;
			tl.append(sl)
		#}
		if state == 0: #{
			sl = sl + c;
		#}
		if state >= 1: #{
			tl[state-1] = tl[state-1] + c;
		#}
	#}
	return (sl, tl);
#}

def parse_input(line):
	sentence = []
	escaped = False;
	reading_word = False;
	lu = ''

	for c in line: #{
		if c == '\\': #{
			escaped = True;
			lu = lu + c;
			c = sys.stdin.read(1);
		#}
		if c == '^': #{
			reading_word = True;
		#}
		if c == '$' and escaped == False: #{
			lu = lu + c;
			reading_word = False;
			(sl, tl) = process_biltrans_unit(lu)
			sentence.append(tl)
			lu = '';
		#}
		if c != '\\' and escaped == True: #{
			escaped = False;
		#}
		if c.isspace(): #{
			if reading_word == False: #{
				continue;
			#}
		#}
		if reading_word: #{
			lu = lu + c;
		#}
	#}sys.stdout.writesys.stdout.write
	return sentence
#}
def process(sentence, start, out): #{
	global t
	global lineno
	if start >= len(sentence): #{
		sys.stdout.write ('.[][' + str(lineno) + " " + str(t) + '].[]\t')
		for s in out:
			sys.stdout.write("^" + s + "$ ");
		print ''
		t += 1
		return;
	#}
	tokens = sentence[start]
	for token in tokens: #{
		out.append(token)
		process(sentence, start + 1, out);
		del out[-1]
	#}
#}

while True: #{
	lineno
	string = sys.stdin.readline().rstrip();
	if string == "":
		break;
	
	tokens = parse_input(string)
	# print map(len, tokens)
	# print reduce(mul, map(len, tokens))

	process(tokens, 0, [])
	lineno += 1
	t = 0
#}
