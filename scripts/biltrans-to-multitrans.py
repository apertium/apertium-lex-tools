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
	for c in lu: #{
		if c == '^' or c == '$': #{
			continue;
		#}
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
			new_paths[path] = sents[path] + '^' + sl + tl[state] + '$';
		#}
	#}
	

	return new_paths;
#}

while c: #{
	if c == '^': #{
		reading_word = True;
	#}
	if c == '$': #{
		lu = lu + c;
		output_sentences = process_biltrans_unit(lu, output_sentences);
		reading_word = False;
		lu = '';		
	#}
	if c.isspace(): #{
		if c == '\n': #{
			i = 0;
			for sentence in output_sentences: #{
				print '.[][' + str(lineno) + ' ' + str(i) + '].[]' , output_sentences[sentence];
				i = i + 1;
			#}
			lineno = lineno + 1;

			output_sentences = {};
			output_sentences[''] = '';
			
		elif reading_word == False: #{
			for sentence in output_sentences: #{
				output_sentences[sentence] = output_sentences[sentence] + c;
			#}
		#}
	#}
	if reading_word: #{
		lu = lu + c;
	#}
	c = sys.stdin.read(1);
#}
