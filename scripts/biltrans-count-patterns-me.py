#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
from collections import defaultdict
import common
import biltrans_count_common as BCC

# Input:
#        a) Frequency lexicon
#        b) Biltrans output
#        c) Disambiguated biltrans output

features = {} # features[(sl, ['a', 'list'], tl)] = 3

sl_tl, sl_tl_defaults, indexes = BCC.read_frequencies(sys.argv[1])

class Counter(BCC.BiltransCounter):
	tokenizer = 'regex'
	line_ids = False
	count_ngrams = True
	max_ngrams = 3

	def process_lu(self, sl, tl, idx, cur_sl_row, frac_count=0):
		global sl_tl, sl_tl_defaults, features, indexes
		sym = '-' if tl == sl_tl_defaults[sl] else '+'
		print(sym, sl, sl_tl_defaults[sl], tl, file=sys.stderr)
		BCC.features_and_outline(self.ngrams, sl, tl, sl_tl, features,
								 indexes)
		self.clear_ngrams()

c = Counter()
c.read_files(sys.argv[2], # File with ambiguous biltrans output
			 sys.argv[3]) # File with disambiguated biltrans output

for feature in features:
	print(features[feature] , '\t' , feature, file=sys.stderr)
