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

# Then we want to make a list ofpython $SCRIPTS/ngram-count-patterns-maxent.py $TRAIN/$CORPUS.lex.$SL-$TL $TRAIN/$CORPUS.candidates.$SL-$TL 2>ngrams > events n-grams around the source words, with which target word they want, and the freq.

# ngrams[ngram][tl_word] = freq

# 5 	Please<vblex><inf> rise<n> ,<cm> then<adv> ,<cm> for<pr> this<det><dem> minute<n> 's<gen> silence<n> .<sent>
#5 	Please<vblex><inf>/Complacer<vblex><inf> rise<n><sg>/aumento<n><m><sg> ,<cm>/,<cm> then<adv>/entonces<adv> ,<cm>/,<cm> for<pr>/para<pr>/durante<pr> this<det><dem><sg>/este<det><dem><GD><sg> minute<n><sg>/minuto<n><m><sg> '<apos>/'<apos> *s/*s silence<n><sg>/silencio<n><m><sg> .<sent>/.<sent>
#5 	Invitar<vblex> a<pr> todo<prn><tn> a<pr> que<cnjsub> prpers<prn><pro> poner<vblex> de<pr> pie<n> para<pr> guardar<vblex><inf> uno<det><ind> minuto<n> de<pr> silencio<n> .<sent>
#5 	0-0 4-2 5-3 8-1 9-5 10-6 12-7 13-8 14-9 15-10
#-------------------------------------------------------------------------------

THRESHOLD = 0
if len(sys.argv) not in [3, 4]: #{
	print 'count-patterns.py <lex> <extracted> [threshold]'
	sys.exit(-1);
#}

if len(sys.argv) == 4:
	THRESHOLD = int(sys.argv[3])

MAX_NGRAMS = 3;
cur_line = 0;

sl_tl_defaults = {}; 
sl_tl = {};
ngrams = {};

meevents = {}; # events[slword][counter] = [feat, feat, feat];
meoutcomes = {}; # meoutcomes[slword][counter] = tlword;
event_counter = 0;

features = {}; # features[(slword, ['a', 'list'], tlword)] = 3
feature_counter = 0;

indexes = {};
trad_counter = {}; 

def wrap (x):
	return '^' + x + '$'

for line in file(sys.argv[1]).readlines(): #{
	if len(line) < 1: #{
		continue;
	#}
	w = int(line.decode('utf-8').split(' ')[0])
	if w < THRESHOLD:
		continue;

	row = common.tokenize_tagger_line(line.decode('utf-8'));
	sl = wrap(row[0]).lower();
	tl = wrap(row[1].strip()).lower();
	if tl[1] == '*':
		tl = tl[:-3] + '$'
	
	if sl not in sl_tl: #{
		sl_tl[sl] = [];
	#}
	if sl not in trad_counter: #{
		trad_counter[sl] = 0;
	#}
	if line.count('@') > 0: #{
		sl_tl_defaults[sl] = tl;
	sl_tl[sl].append(tl);
	indexes[(sl, tl)] = trad_counter[sl];
	trad_counter[sl] = trad_counter[sl] + 1;
	
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
			if len(cur_bt_row[i]['tls']) > 1: #{
				for al in cur_al_row: #{
					al_sl = int(al.split('-')[1]);
					al_tl = int(al.split('-')[0]);
					if al_sl != i: #{
						continue;
					#}

					tlword = wrap(cur_tl_row[al_tl].lower());
					slword = wrap(slword.lower());

					if tlword[1] == '*' or slword[1] == '*':
						continue;
					
					if slword not in sl_tl_defaults: #{
#						print >>sys.stderr, 'WARNING: "' + slword + '" not in sl_tl_defaults, skipping';
						continue;
					#}
					if (slword, tlword) not in indexes: #{
#						print >>sys.stderr, 'WARNING: pair (%s, %s) not found in index' % (slword, tlword);
						continue;
					#}
#					if tlword !=  sl_tl_defaults[slword]: #{
#						print >>sys.stderr, '+' , slword , sl_tl_defaults[slword] , tlword;
#					else: #{
#						print >>sys.stderr, '-' , slword , sl_tl_defaults[slword] , tlword;
#					#}
#					print >>sys.stderr, cur_sl_row;
					for j in range(1, MAX_NGRAMS): #{
#						print >>sys.stderr, cur_sl_row[i] , cur_sl_row[i-j:i+1]
#						print >>sys.stderr, cur_sl_row[i] , cur_sl_row[i:i+j+1]
#						print >>sys.stderr, cur_sl_row[i] , cur_sl_row[i-j:i+j+1]


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
					#print ',' , len(ngrams[slword]);
					if slword not in meevents: #{
						meevents[slword] = {};
					#}
					if slword not in meoutcomes: #{
						meoutcomes[slword] = {};
					#}
					if event_counter not in meevents: #{
						meevents[slword][event_counter] = [];
					#}
					if event_counter not in meoutcomes[slword]: #{
						meoutcomes[slword][event_counter] = '';
					#}
					for ni in ngrams[slword]: #{
						if ni not in features: #{
							feature_counter = feature_counter + 1;
							features[ni] = feature_counter;
						#}
						meevents[slword][event_counter].append(features[ni]);
						#meevents[slword][event_counter].append(feat);
						meoutcomes[slword][event_counter] = tlword;
						
					#}
					del ngrams;
					ngrams = {};
					if len(sl_tl[slword]) < 2: #{
						continue;
					#}
					for event in meevents[slword]: #{
						outline = str(indexes[(slword, meoutcomes[slword][event])]) + ' # ';
						for j in range(0,  len(sl_tl[slword])): #{
							for feature in meevents[slword][event]: #{
								outline = outline + str(feature) + ':' + str(j) + ' ';
							#}
							outline = outline + ' # '
						#}
						print slword , '\t', len(sl_tl[slword]),'\t', outline;
					#}
					del meevents;
					del meoutcomes;
					meevents = {};
					meoutcomes = {};

#					for f in features: #{
#						print >>sys.stderr, features[f] , f;
#					#}

				#}

#				for j in range(0, MAX_NGRAMS): #{
#					print cur_sl_row[i-j:i+1];
#					print cur_sl_row[i:i+j];
#				#}
				#print ngrams[slword];
			#}	
			i = i + 1;

		#}

		cur_line = 0;
		event_counter = event_counter + 1;
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

for feature in features: #{
	print >> sys.stderr, features[feature] , '\t' , feature;
#}

sys.exit(-1);

for slword in meevents: #{
	if len(sl_tl[slword]) < 2: #{
		continue;
	#}
	for event in meevents[slword]: #{
		outline = str(indexes[(slword, meoutcomes[slword][event])]) + ' # ';
		for j in range(0,  len(sl_tl[slword])): #{
			for feature in meevents[slword][event]: #{
				outline = outline + str(feature) + ':' + str(j) + ' ';
			#}
			outline = outline + ' # '
		#}
		print slword , '\t', len(sl_tl[slword]),'\t', outline;
	#}
#}
