#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs;
import common;

if len(sys.argv) < 2: #{
	print('extact-sentences.py <phrasetable> <biltrans>');
	sys.exit(-1);
#}

phrase_table = open(sys.argv[1]);
biltrans_out = open(sys.argv[2]);

def ambiguous(bt): #{
	# legislation<n><sg>/legislación<n><f><sg>/ordenamiento<n><m><sg>
	
	ambig = False;
	for w in bt.split(' '): #{
		if w.count('/') > 1: #{
			if w.count('<n>') > 0 or w.count('<adj>') > 0 or w.count('<vblex>') > 0: #{
				return True;
			#}
		#}
	#}

	return ambig;
#}

reading = True;
lineno = 0;
total_valid = 0;

while reading: #{
	lineno = lineno + 1;
	pt_line = phrase_table.readline().strip();	
	bt_line = biltrans_out.readline().strip();

	if bt_line == '' and pt_line == '': #{
		reading = False;
	#}

	if not ambiguous(bt_line): #{
#		print(lineno, ' not ambiguous.', file=sys.stderr);
		continue;
	#}



	row = pt_line.split('|||');
	print (common.tokenize_tagger_line(row[0]));
	bt = bt_line.split();
	sl = row[1].strip();
	tl = row[0].strip();
	aliniaments = row[2].strip();

	bt_row = bt_line.split(' ');
	sl_row = sl.split(' ');
	tl_row = tl.split(' ');

	if len(sl_row) < 2 and len(tl_row) < 2: #{
		continue;
	#}

	# Check that the number of words in the lexical transfer, and in the phrasetable matches up
	if len(sl_row) != len(bt_row): #{
#		print(lineno, '!!!: ', len(sl_row) , '!=', len(bt_row), file=sys.stderr);
#		print('\tsl:', sl, file=sys.stderr);
#		print('\tbt:', bt, file=sys.stderr);
		continue;
	#}

#	print(lineno, ' ambiguous', file=sys.stderr);

	words = {};	
	i = 0;

	# Here we collect a set of SL words, with their correspondences in the bilingual
	# dictionary, and the word they have been aligned with in the target.
        # e.g.  words[0] = ('sl', ['bt1', 'bt2', ...], 'tl')
	for i in range(0, len(sl_row)): #{
		tl_vals = [];
		for j in aliniaments.split(' '): #{
			ament = j.split('-');
			if int(ament[1]) == i: #{
				t_ament = int(ament[0]);
				if t_ament > len(tl_row): #{
#					print(lineno, '!!!: ', t_ament, '>', len(tl_row), file=sys.stderr);
#					print('\tsl:', sl, file=sys.stderr);
#					print('\tbt:', bt, file=sys.stderr);
#					print('\tal:', aliniaments, file=sys.stderr);
					continue;
				#}
				tl_vals.append(tl_row[t_ament]);
			#}
		#}
		bt_vals = bt_row[i].split('/')[1:];
		words[i] = (sl_row[i], bt_vals, tl_vals);

#		print('\t', words[i][0],  bt_vals, tl_vals, file=sys.stderr);
	#}

	current_ambig_words = {};

	valid = True;
	i = 0;
	
	#
	for word in words: #{
		# If the word is ambiguous
		if len(words[word][1]) > 1: #{
			current_ambig_words[i] = words[word];
	
			# Check to see if the TL possibilities are found in the lexical 
			# transfer output.
			for tlw in words[word][2]: #{
				found = False;
				for btw in words[word][1]: #{
					needle = tlw.split('<')[0] + '<';
					if btw.count(needle) > 0: #{
						found = True;
					#}
				#}	
				if not found: #{
					print('!!!: Missing:' , tlw, 'not found for', words[word][0], file=sys.stderr);
#					print('\tw:' , word, file=sys.stderr);
					valid = False;
				#}
			#}			
		#}
		i = i + 1;
	#}

	if not valid: #{
		continue;
	#}

	# Resumption<n> of<pr> the<def><def> session<n> 
	# Resumption<n><sg>/Reanudación<n><f><sg> of<pr>/de<pr> the<det><def><sp>/el<det><def><GD><ND> session<n><sg>/sesión<n><f><sg> 
	# Reanudación<n> de<pr> el<det><def> periodo<n> de<pr> sesión<n> 
	# 0-0 1-1 2-2 5-3


	print(lineno, '\t' + sl);
	print(lineno, '\t' + bt_line);
	print(lineno, '\t' + tl);
	print(lineno, '\t' + aliniaments);
	print('-------------------------------------------------------------------------------');

	
	total_valid = total_valid + 1;
#}

print('total:', lineno, file=sys.stderr);
print('valid:', total_valid, '(' + str((total_valid/lineno)*100) + '%)', file=sys.stderr);
