#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);


output_sentences = {};
output_sentences[''] = '';
reading_word = False;
lineno = 1;
lu = '';
c = sys.stdin.read(1);

def process_biltrans_unit(lu, sents): #{
	new_paths = {};

	state = 0;
	sl = '';
	tl = {};
	for c in lu[1:-1]: #{ 
		#^worth<n><sg>/valor<n><m><sg>$ ^\$<mon>/\$<mon>$^20<num>/20<num>$^*m/*m$ 
		#print c , sl , tl;
		if c == '/': #{
			state = state + 1;
			if state not in tl: #{
				tl[state] = '';
			#}
		#}
		if state == 0: #{
			sl = sl + c;
		#}
		if state >= 1: #{
			tl[state] = tl[state] + c;
		#}
	#}

	if len(tl) > 1: #{
		for trad in tl: #{
			for path in sents: #{
				new_paths[path + tl[trad]] = sents[path] + '^' + sl + tl[trad] + '$';
			#}
		#}
	else: #{
		for path in sents: #{
			if state not in tl: #{
				print >> sys.stderr, 'ERROR: ';
				print >> sys.stderr, sl ; 
				print >> sys.stderr, tl ; 
			#}
			new_paths[path] = sents[path] + '^' + sl + tl[state] + '$';
		#}
	#}
	

	return new_paths;
#}

escaped = False;
seen_newline = True;
cur_id = '';
while c: #{
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
		new_paths = process_biltrans_unit(lu, output_sentences);
		del output_sentences;
		output_sentences = new_paths;
		reading_word = False;
		lu = '';		
	#}
	if c != '\\' and escaped == True: #{
		escaped = False;
	#}
	if c.isspace(): #{
		seen_newline = False;
		if c == '\n': #{
			print >> sys.stderr, 'output_sentences: ', len(output_sentences);
			i = 0;
			for sentence in output_sentences: #{
				#print '.[][' + str(lineno) + ' ' + str(i) + ' ' + cur_id +'].[]\t' , output_sentences[sentence];
				print '.[][' + cur_id + ' ' + str(i) + '].[]\t' , output_sentences[sentence];
				i = i + 1;
			#}
			lineno = lineno + 1;

			output_sentences = {};
			output_sentences[''] = '';
			seen_newline = True;
			cur_id = '';
			
		elif reading_word == False: #{
			for sentence in output_sentences: #{
				output_sentences[sentence] = output_sentences[sentence] + c;
			#}
		#}
	#}
	if reading_word: #{
		lu = lu + c;
	#}
	if seen_newline and c != '\n': #{
		cur_id = cur_id + c;
	#}
	c = sys.stdin.read(1);
#}
