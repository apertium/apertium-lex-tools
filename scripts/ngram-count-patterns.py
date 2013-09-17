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

if len(sys.argv) < 3: #{
	print ('count-patterns.py <lex> <extracted> <crispiness threshold>');
	sys.exit(-1);
#}

MAX_NGRAMS = 3;

crisphold = float(sys.argv[3]);
cur_line = 0;

sl_tl_defaults = {}; 
sl_tl = {};
ngrams = {};

for line in file(sys.argv[1]).readlines(): #{
	if len(line) < 1: #{
		continue;
	#}
	row = common.tokenize_tagger_line(line.decode('utf-8'));
	sl = wrap(row[0]);
	tl = wrap(row[1]);
	if tl[1] == '*':
		tl = tl[:-3] + '$'
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
lineno = 0
for line in file(sys.argv[2]).readlines(): #{
	lineno += 1
	line = line.strip().decode('utf-8');	
	if line[0] == '-': #{
		try:
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
				if len(cur_bt_row[i]['tls']) > 1: #{
					for al in cur_al_row: #{
						al_sl = int(al.split('-')[1]);
						al_tl = int(al.split('-')[0]);
						if al_sl != i: #{
							continue;
						#}
						tlword = wrap(cur_tl_row[al_tl]);
						slword = wrap(slword);

						if slword not in sl_tl_defaults: #{
							print >>sys.stderr, 'WARNING: "' + slword + '" not in sl_tl_defaults, skipping';
							continue;
						#}

						for j in range(1, MAX_NGRAMS): #{

							pregram = ' '.join(map(wrap, cur_sl_row[i-j:i+1]));
							postgram = ' '.join(map(wrap, cur_sl_row[i:i+j+1]));
							roundgram = ' '.join(map(wrap, cur_sl_row[i-j:i+j+1]));

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
		except:
			print >>sys.stderr, "error in line", lineno
		cur_line = 0;
		#print line;	
		continue;
	#}	
	
	line = line.split('\t')[1];

	if cur_line == 0: #{
		cur_sl_row = common.tokenize_tagger_line(line)
	elif cur_line == 1: #{
		cur_bt_row = common.tokenize_biltrans_line(line)
	elif cur_line == 2: #{
		cur_tl_row = common.tokenize_tagger_line(line)
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

		#> If for each of the rules we include
		#> the amount of time the translation is seen with that pattern over the
		#> total, we get a number we can try as a threshold. e.g. > 0.6 >0.7 >0.8
		#> etc.  (>0.6 would be the same as 2/3 of the time the alternative
		#> translation is seen with that ngram, and 1/3 of the time the default
		#> translation is). I think this would be easier to explain than the magic
		#> number I came up with.
		#
		#I see this as a way to define how "crispy" the decisions are. I think it 
		#would be better to express this as a ratio: the ratio of the times the 
		#alternative translation is seen to the number of times the defaullt 
		#translation is seen with that n-gram.
		#
		#It would be "2" in this case: the alternative is seen twice as often as 
		#the default.
		
		for tl in ngrams[sl][ngram]: #{
			crispiness = 0.0;
			default = sl_tl_defaults[sl];
			alt_crisp = float(ngrams[sl][ngram][tl]) / float(total);
			def_crisp = 1.0;
			if default in ngrams[sl][ngram]: #{
				def_crisp = float(ngrams[sl][ngram][default] / float(total));
			#}
			weight = float(ngrams[sl][ngram][tl]) / float(total);
			crispiness = alt_crisp/def_crisp;

			#print '%%%' , crispiness , alt_crisp , def_crisp , tl , default , ngrams[sl][ngram] ; 
			
			if crispiness < crisphold: #{
				print '-', crispiness , weight , total, max_freq, ngrams[sl][ngram][tl], '\t'+ sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][tl]);
			else: #{

				print '+', crispiness , weight , total, max_freq, ngrams[sl][ngram][tl], '\t' +  sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][current_tl]);
			#}

		#}
	#}
#}
