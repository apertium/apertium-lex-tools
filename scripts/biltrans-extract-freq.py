#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import common
import biltrans_count_common as BCC
from collections import defaultdict

# Input:
#        a) Biltrans output
#        b) Disambiguated biltrans output

# The sl-tl possible combinations
sl_tl = defaultdict(lambda: defaultdict(lambda: 0))

class Counter(BCC.BiltransCounter):
	tokenizer = 'biltrans'
	line_ids = False

	def process_lu(self, sl, tl, idx, cur_sl_row, frac_count=0):
		global sl_tl
		sl_tl[sl][tl] += 1

c = Counter()
c.read_files(sys.argv[1], # File with ambiguous biltrans output
			 sys.argv[2]) # File with disambiguated biltrans output

for sl in sl_tl:
	newtl = sorted(sl_tl[sl], key=lambda x: sl_tl[sl][x])
	newtl.reverse()
	first = True
	for tl in newtl:
		if first:
			print(sl_tl[sl][tl] , sl , tl , '@')
			first = False
		else:
			print(sl_tl[sl][tl] , sl , tl)
