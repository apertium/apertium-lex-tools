#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;
import common

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

# Read the corpus, make a note of all ambiguous words, their frequency and their possible translations

# sl_tl[sl_word][tl_word] = tl_freq

# Then we want to make a list of n-grams around the source words, with which target word they want, and the freq.

# ngrams[ngram][tl_word] = freq

# 5 	Please<vblex><inf> rise<n> ,<cm> then<adv> ,<cm> for<pr> this<det><dem> minute<n> 's<gen> silence<n> .<sent>
#5 	Please<vblex><inf>/Complacer<vblex><inf> rise<n><sg>/aumento<n><m><sg> ,<cm>/,<cm> then<adv>/entonces<adv> ,<cm>/,<cm> for<pr>/para<pr>/durante<pr> this<det><dem><sg>/este<det><dem><GD><sg> minute<n><sg>/minuto<n><m><sg> '<apos>/'<apos> *s/*s silence<n><sg>/silencio<n><m><sg> .<sent>/.<sent>
#5 	Invitar<vblex> a<pr> todo<prn><tn> a<pr> que<cnjsub> prpers<prn><pro> poner<vblex> de<pr> pie<n> para<pr> guardar<vblex><inf> uno<det><ind> minuto<n> de<pr> silencio<n> .<sent>
#5 	0-0 4-2 5-3 8-1 9-5 10-6 12-7 13-8 14-9 15-10
#-------------------------------------------------------------------------------

def wrap (x):
	return '^' + x + '$'

MAX_NGRAMS = 3;

cur_line = 0;
lineno = 0;
sl_tl = {};
ngrams = {};

cur_sl_row = [];
cur_tl_row = [];
cur_bt_row = [];
cur_al_row = [];

if len(sys.argv) < 2: #{
	print 'extract-freq-lexicon.py <candidate sent> [threshold]';
	sys.exit(-1);
#}


for line in file(sys.argv[1]).readlines(): #{
	line = line.strip().decode('utf-8');	
	lineno += 1
	if line[0] == '-': #{
		try:
			# Read the corpus, make a note of all ambiguous words, their frequency and their possible translations
			#
			# sl_tl[sl_word][tl_word] = tl_freq
			i = 0;
			for slword in cur_sl_row: #{
				if len(cur_bt_row[i]['tls']) > 1: #{
					for al in cur_al_row: #{
						al_sl = int(al.split('-')[1]);
						al_tl = int(al.split('-')[0]);
						if al_sl != i: #{
							continue;
						#}
						tlword = cur_tl_row[al_tl];
						slword = slword;
						if slword not in sl_tl: #{
							sl_tl[slword] = {};
						#}
						if tlword not in sl_tl[slword]: #{
							sl_tl[slword][tlword] = 0;
						#}
						sl_tl[slword][tlword] = sl_tl[slword][tlword] + 1;

						# print '+' , slword , tlword , sl_tl[slword][tlword], lineno;
					#}
				#}	
				i = i + 1;
			#}

			cur_line = 0;
		except:
			print >>sys.stderr, "error in line", lineno;
		#print line;	
		continue;
	#}	
	
	line = line.split('\t')[1];

	if cur_line == 0: #{
		cur_sl_row = common.tokenize_tagger_line(line);
	elif cur_line == 1: #{
		cur_bt_row = common.tokenize_biltrans_line(line);
	elif cur_line == 2: #{
		cur_tl_row = common.tokenize_tagger_line(line);
	elif cur_line == 3:  #{
		cur_al_row = line.split(' ');
	#}

	cur_line = cur_line + 1;
#}

for sl in sl_tl: #{

	newtl = sorted(sl_tl[sl], key=lambda x: sl_tl[sl][x])
	newtl.reverse()
	first = True;
	for tl in newtl: #{
		if first: #{
			print sl_tl[sl][tl] , wrap(sl) , wrap(tl) , '@';
			first = False
		else: #{
			print sl_tl[sl][tl] , wrap(sl) , wrap(tl);
		#}
	#}
#}
