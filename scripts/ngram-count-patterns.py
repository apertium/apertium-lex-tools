#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

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

if len(sys.argv) < 2: #{
	print 'count-patterns.py <lex> <extracted>';
	sys.exit(-1);
#}

MAX_NGRAMS = 3;

cur_line = 0;

sl_tl_defaults = {}; 
sl_tl = {};
ngrams = {};

for line in file(sys.argv[1]).readlines(): #{
	if len(line) < 1: #{
		continue;
	#}
	row = line.decode('utf-8').split(' ');
	sl = row[1];
	tl = row[2];
	if line.count('@') > 0: #{
		sl_tl_defaults[sl] = tl;
	else: #{
		sl_tl[sl] = tl;
	#}
#}

cur_sl_row = [];
cur_tl_row = [];
cur_bt_row = [];
cur_al_row = [];

for line in file(sys.argv[2]).readlines(): #{
	line = line.strip().decode('utf-8');	
	if line[0] == '-': #{
#		print len(cur_sl_row), len(cur_tl_row), len(cur_bt_row), len(cur_al_row);	
#		print cur_sl_row;
#		print cur_bt_row;
#		print cur_tl_row;
#		print cur_al_row;
#
		# Read the corpus, make a note of all ambiguous words, their frequency and their possible translations
		#
		# sl_tl[sl_word][tl_word] = tl_freq
		i = 0;
		for slword in cur_sl_row: #{
			if cur_bt_row[i].count('/') > 1: #{
				for al in cur_al_row: #{
					al_sl = int(al.split('-')[1]);
					al_tl = int(al.split('-')[0]);
					if al_sl != i: #{
						continue;
					#}
					tlword = cur_tl_row[al_tl].lower().split('>')[0] + '>';
					slword = slword.lower().split('>')[0] + '>';

					if tlword !=  sl_tl_defaults[slword]: #{
						print >>sys.stderr, '+' , slword , sl_tl_defaults[slword] , tlword;
					else: #{
						print >>sys.stderr, '-' , slword , sl_tl_defaults[slword] , tlword;
					#}
					print >>sys.stderr, cur_sl_row;
					for j in range(1, MAX_NGRAMS): #{
						print >>sys.stderr, cur_sl_row[i] , cur_sl_row[i-j:i+1]
						print >>sys.stderr, cur_sl_row[i] , cur_sl_row[i:i+j+1]
						print >>sys.stderr, cur_sl_row[i] , cur_sl_row[i-j:i+j+1]

						pregram = ' '.join(cur_sl_row[i-j:i+1]);
						postgram = ' '.join(cur_sl_row[i:i+j+1]);
						roundgram = ' '.join(cur_sl_row[i-j:i+j+1]);

						if slword not in ngrams: #{
							ngrams[slword] = {};
						#}
						if pregram not in ngrams[slword]: #{
							ngrams[slword][pregram] = {};
						#}
						if postgram not in ngrams[slword]: #{
							ngrams[slword][postgram] = {};
						#}
						if roundgram not in ngrams[slword]: #{
							ngrams[slword][roundgram] = {};
						#}
						if tlword not in ngrams[slword][pregram]: #{
							ngrams[slword][pregram][tlword] = 0;
						#}
						if tlword not in ngrams[slword][postgram]: #{
							ngrams[slword][postgram][tlword] = 0;
						#}
						if tlword not in ngrams[slword][roundgram]: #{
							ngrams[slword][roundgram][tlword] = 0;
						#}

						ngrams[slword][pregram][tlword] = ngrams[slword][pregram][tlword] + 1;
						ngrams[slword][postgram][tlword] = ngrams[slword][postgram][tlword] + 1;
						ngrams[slword][roundgram][tlword] = ngrams[slword][roundgram][tlword] + 1;
					#}
				#}

#				for j in range(0, MAX_NGRAMS): #{
#					print cur_sl_row[i-j:i+1];
#					print cur_sl_row[i:i+j];
#				#}
			#}	
			i = i + 1;
		#}

		cur_line = 0;
		#print line;	
		continue;
	#}	
	
	line = line.split('\t')[1];

	if cur_line == 0: #{
		cur_sl_row = line.split(' ');
	elif cur_line == 1: #{
		cur_bt_row = line.split(' ');
	elif cur_line == 2: #{
		cur_tl_row = line.split(' ');
	elif cur_line == 3:  #{
		cur_al_row = line.split(' ');
	#}

	cur_line = cur_line + 1;
#}

for sl in ngrams: #{

	for ngram in ngrams[sl]: #{
		total = 0;
		max_freq = -1;	
		current_tl = '';
		for tl in ngrams[sl][ngram]: #{
			if ngrams[sl][ngram][tl] > max_freq: #{
				max_freq = ngrams[sl][ngram][tl];
				current_tl = tl;
			#}
			total = total + ngrams[sl][ngram][tl];
		#}
		for tl in ngrams[sl][ngram]: #{
			if tl == current_tl and tl not in sl_tl_defaults[sl] and total != max_freq: #{
				mf = max_freq / 2;
				if (total - max_freq) < mf:  #{
					print '@', total, max_freq, ngrams[sl][ngram][tl], '\t' + sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][current_tl]);
				else: #{	
					print '+', total, max_freq, ngrams[sl][ngram][tl], '\t' +  sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][current_tl]);
				#}
				continue;
			#}
			if tl == current_tl and tl not in sl_tl_defaults[sl] and total == max_freq: #{
				print '~', total, max_freq, ngrams[sl][ngram][tl], '\t'+ sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][current_tl]);
				continue;
			#}
			print '-', total, max_freq, ngrams[sl][ngram][tl], '\t'+ sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][tl]);
		#}
	#}
#}
