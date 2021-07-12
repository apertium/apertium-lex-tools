#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import biltrans_count_common as BCC

# Input:
#        a) Frequency lexicon

# 0.9983989491 kemennadur<n> précepte<n> @
# 0.0016010509 kemennadur<n> directive<n>
# 2.9999998997 a-led<adv> horizontal<adv> @
# 0.0000001004 a-led<adv> à~plat<adv>
# 20.4545497200 pazenn<n> étape<n> @
# 2.5454502800 pazenn<n> marche<n>
#

#        b) Biltrans output

# 56011   ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$


#        c) Disambiguated biltrans output

#.[][56011 0].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$ 0.9917274061    |@|
#.[][56011 1].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$       0.0082725939    ||

#	 d) Crispiness threshold

cur_line = 0
crisphold = 3.0  # Default
only_max = True
#only_max = False

if len(sys.argv) == 5:
	crisphold = float(sys.argv[4])
	print('crisp:', crisphold, file=sys.stderr)

# First read in the frequency defaults

sl_tl, sl_tl_defaults, _ = BCC.read_frequencies(sys.argv[1])

print('Reading...', file=sys.stderr)
sys.stderr.flush()

class Counter(BCC.BiltransCounter):
	tokenizer = 'biltrans'
	line_ids = True
	count_ngrams = True
	max_ngrams = 3

c = Counter()
c.read_files(sys.argv[2], # File with ambiguous biltrans output
			 sys.argv[3]) # File with disambiguated biltrans output
ngrams = c.ngrams

print('Caching counts...', file=sys.stderr)
for sl in ngrams:
	for ngram in ngrams[sl]:
		for tl in ngrams[sl][ngram]:
			print('%.10f\t%s\t%s\t%s' % (ngrams[sl][ngram][tl], ngram, sl, tl))

print('\n', file=sys.stderr)
