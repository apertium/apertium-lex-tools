#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

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

def parse_input():
	string = sys.stdin.readline().rstrip();
	sentence = []
	escaped = False;
	reading_word = False;
	lu = ''

	for c in string: #{
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

t = 0
def process(sentence, start, out): #{
	global t
	if start >= len(sentence):
		print '.[][' + str(t) + '][].'
		for s in out:
			sys.stdout.write(s + " ");
		print ''
		t += 1
		return;
	
	tokens = sentence[start]
	for token in tokens: #{
		out.append(token)
		process(sentence, start + 1, out);
		del out[-1]
	#}
#}
tokens = parse_input()
process(tokens, 0, [])

