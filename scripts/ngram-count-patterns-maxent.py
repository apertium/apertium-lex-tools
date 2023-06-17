#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
from collections import defaultdict

# Read the corpus, make a note of all ambiguous words, their frequency and their possible translations

# sl_tl[sl_word][tl_word] = tl_freq

# Then we want to make a list ofpython $SCRIPTS/ngram-count-patterns-maxent.py $TRAIN/$CORPUS.lex.$SL-$TL $TRAIN/$CORPUS.candidates.$SL-$TL 2>ngrams > events n-grams around the source words, with which target word they want, and the freq.

# ngrams[ngram][tl_word] = freq

# 5 	Please<vblex><inf> rise<n> ,<cm> then<adv> ,<cm> for<pr> this<det><dem> minute<n> 's<gen> silence<n> .<sent>
#5 	Please<vblex><inf>/Complacer<vblex><inf> rise<n><sg>/aumento<n><m><sg> ,<cm>/,<cm> then<adv>/entonces<adv> ,<cm>/,<cm> for<pr>/para<pr>/durante<pr> this<det><dem><sg>/este<det><dem><GD><sg> minute<n><sg>/minuto<n><m><sg> '<apos>/'<apos> *s/*s silence<n><sg>/silencio<n><m><sg> .<sent>/.<sent>
#5 	Invitar<vblex> a<pr> todo<prn><tn> a<pr> que<cnjsub> prpers<prn><pro> poner<vblex> de<pr> pie<n> para<pr> guardar<vblex><inf> uno<det><ind> minuto<n> de<pr> silencio<n> .<sent>
#5 	0-0 4-2 5-3 8-1 9-5 10-6 12-7 13-8 14-9 15-10
#-------------------------------------------------------------------------------

if len(sys.argv) < 2:
	print('count-patterns.py <lex> <extracted>')
	sys.exit(-1)


MAX_NGRAMS = 3

cur_line = 0

sl_tl_defaults = {}
sl_tl = defaultdict(list)
ngrams = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: 0)))

meevents = defaultdict(lambda: defaultdict(list)) # events[slword][counter] = [feat, feat, feat]
meoutcomes = defaultdict(lambda: defaultdict(lambda: '')) # meoutcomes[slword][counter] = tlword
event_counter = 0

features = {} # features[(slword, ['a', 'list'], tlword)] = 3
feature_counter = 0

indexes = {}
trad_counter = defaultdict(lambda: 0)

infile = open(sys.argv[1])

for line in infile
	if len(line) < 1:
		continue

	row = line.split(' ')
	sl = row[1]
	tl = row[2].strip()

	if line.count('@') > 0:
		sl_tl_defaults[sl] = tl
		sl_tl[sl].append(tl)
		indexes[(sl, tl)] = trad_counter[sl]
		trad_counter[sl] += 1
	else:
		sl_tl[sl].append(tl)
		indexes[(sl, tl)] = trad_counter[sl]
		trad_counter[sl] += 1

infile.close()

cur_sl_row = []
cur_tl_row = []
cur_bt_row = []
cur_al_row = []

infile = open(sys.argv[2])

for line in infile:
	line = line.strip()
	if line.startswith('-'):
		# Read the corpus, make a note of all ambiguous words, their frequency and their possible translations
		#
		# sl_tl[sl_word][tl_word] = tl_freq
		i = 0
		for slword in cur_sl_row:
			if cur_bt_row[i].count('/') > 1:
				for al in cur_al_row:
					al_sl = int(al.split('-')[1])
					al_tl = int(al.split('-')[0])
					if al_sl != i:
						continue


					tlword = cur_tl_row[al_tl]
					slword = slword

					if slword not in sl_tl_defaults:
#						print('WARNING: "' + slword + '" not in sl_tl_defaults, skipping', file=sys.stderr)
						continue

					if (slword, tlword) not in indexes:
#						print('WARNING: pair (%s, %s) not found in index' % (slword, tlword), file=sys.stderr)
						continue

#					if tlword !=  sl_tl_defaults[slword]:
#						print('+', slword, sl_tl_defaults[slword], tlword, file=sys.stderr)
#					else:
#						print('-', slword, sl_tl_defaults[slword], tlword, file=sys.stderr)


					for j in range(1, MAX_NGRAMS):
#						print(cur_sl_row[i], cur_sl_row[i-j:i+1], file=sys.stderr)
#						print(cur_sl_row[i], cur_sl_row[i:i+j+1], file=sys.stderr)
#						print(cur_sl_row[i], cur_sl_row[i-j:i+j+1], file=sys.stderr)

						pregram = ' '.join(cur_sl_row[i-j:i+1])
						postgram = ' '.join(cur_sl_row[i:i+j+1])
						roundgram = ' '.join(cur_sl_row[i-j:i+j+1])

						ngrams[slword][pregram][tlword] += 1
						ngrams[slword][postgram][tlword] += 1
						ngrams[slword][roundgram][tlword] += 1

					#print(',', len(ngrams[slword]))
					for ni in ngrams[slword]:
						if ni not in features:
							feature_counter += 1
							features[ni] = feature_counter

						meevents[slword][event_counter].append(features[ni])
						meoutcomes[slword][event_counter] = tlword

					ngrams.clear()
					if len(sl_tl[slword]) < 2:
						continue

					for event in meevents[slword]:
						outline = str(indexes[(slword, meoutcomes[slword][event])]) + ' # '
						for j in range(0, len(sl_tl[slword])):
							for feature in meevents[slword][event]:
								outline += '%s:%s ' % (feature, j)
							outline += ' # '

						print(slword, '\t', len(sl_tl[slword]),'\t', outline)

					meevents.clear()
					meoutcomes.clear()

#					for f in features:
#						print(features[f], f, file=sys.stderr)
#

#				for j in range(0, MAX_NGRAMS):
#					print(cur_sl_row[i-j:i+1])
#					print(cur_sl_row[i:i+j])
#
				#print(ngrams[slword])

			i += 1
		cur_line = 0
		event_counter += 1
		#print(line)
		continue

	line = line.split('\t')[1]

	if cur_line == 0:
		cur_sl_row = line.split(' ')
	elif cur_line == 1:
		cur_bt_row = line.split(' ')
	elif cur_line == 2:
		cur_tl_row = line.split(' ')
	elif cur_line == 3:
		cur_al_row = line.split(' ')

	cur_line += 1

infile.close()

for feature in features:
	print(features[feature], '\t', feature, file=sys.stderr)

sys.exit(-1)

for slword in meevents:
	if len(sl_tl[slword]) < 2:
		continue

	for event in meevents[slword]:
		outline = str(indexes[(slword, meoutcomes[slword][event])]) + ' # '
		for j in range(0, len(sl_tl[slword])):
			for feature in meevents[slword][event]:
				outline += str(feature) + ':' + str(j) + ' '
			outline += ' # '
		print(slword, '\t', len(sl_tl[slword]),'\t', outline)
