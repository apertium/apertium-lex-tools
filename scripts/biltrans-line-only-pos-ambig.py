#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys;

pos = ["<n>", "<vblex>", "<adj>"];

def process_line(l): #{
	global pos;
	w = '';
	in_word = False;
	escaped = False;
	for c in l: #{
		if c == '\\': #{
			escaped = True;
			continue;
		#}
		if c == '^' and escaped == False: #{
			in_word = True;
		#}
		if c == '$' and escaped == False: #{
			word_in_pos = False;
			for p in pos: #{
				if w.count(p) > 0: #{
					word_in_pos = True;
				#}
			#}
			
			if w.count('/') > 1 and word_in_pos == True: #{
				return True;
			#}
			
			w = '';
			in_word = False;
		#}
		if in_word == True: #{
			w = w + c;
		#}
		escaped = False;
	#}
	return False;	
#}

output = False;	

infile = sys.stdin ; 

if len(sys.argv) > 1: #{
	infile = open(sys.argv[1]);
#}

for line in infile.readlines(): #{

	output = process_line(line);
		
	if output == True: #{
		print(line.strip());
	#}
#	else:
#		print(line.strip(), file=sys.stderr);
#}
