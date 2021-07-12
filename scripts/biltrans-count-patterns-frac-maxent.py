#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, math, re, common
from collections import defaultdict
import biltrans_count_common as BCC

# Input:
#	a) Frequency lexicon

# 0.9983989491 kemennadur<n> précepte<n> @
# 0.0016010509 kemennadur<n> directive<n>
# 2.9999998997 a-led<adv> horizontal<adv> @
# 0.0000001004 a-led<adv> à~plat<adv>
# 20.4545497200 pazenn<n> étape<n> @
# 2.5454502800 pazenn<n> marche<n>
#

#	b) Biltrans output

# 56011   ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$


#	c) Disambiguated biltrans output

#.[][56011 0].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$ 0.9917274061    |@|
#.[][56011 1].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$       0.0082725939    ||

sl_tl_defaults = {}
sl_tl = defaultdict(list)

features = {} # features[(slword, ['a', 'list'], tlword)] = 3

indexes = {}
trad_counter = defaultdict(lambda: 0)

# First read in the frequency defaults

for line in open(sys.argv[1]):
	line = line.strip()
	if len(line) < 1:
		continue

	row = common.tokenize_tagger_line(line)
	sl = common.wrap(row[0])
	tl = common.wrap(row[1])
	if tl[1] == '*':
		tl = tl[:-3] + '$'

	indexes[(sl, tl)] = trad_counter[sl]
	trad_counter[sl] += 1
	sl_tl[sl].append(tl)

	if line.count('@') > 0:
		sl_tl_defaults[sl] = tl

class Counter(BCC.BiltransCounter):
	tokenizer = 'biltrans'
	line_ids = True
	count_ngrams = True
	max_ngrams = 3
	biltrans_wrap_lus = True

	def process_lu(self, sl, tl, idx, cur_sl_row, frac_count=0):
		global sl_tl, features, indexes
		BCC.features_and_outline(self.ngrams, sl, tl, sl_tl, features,
								 indexes, frac_count=frac_count)
		self.clear_ngrams()

c = Counter()
c.read_files(sys.argv[2], # File with ambiguous biltrans output
			 sys.argv[3]) # File with disambiguated biltrans output

for feature in features:
	print(features[feature] , '\t' , feature, file=sys.stderr)
