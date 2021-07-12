#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
from collections import defaultdict
import biltrans_count_common as BCC

# Input:
#        a) Biltrans output
# 56011   ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$
#
#        b) Disambiguated biltrans output
#.[][56011 0].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$ 0.9917274061    |@|
#.[][56011 1].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$       0.0082725939    ||
#
#

# The sl-tl possible combinations
sl_tl = defaultdict(lambda: defaultdict(lambda: 0.0))

class Counter(BCC.BiltransCounter):
	tokenizer = 'biltrans'
	line_ids = True

	def process_lu(self, sl, tl, idx, cur_sl_row, frac_count=0):
		global sl_tl
		sl_tl[sl][tl] += frac_count

c = Counter()
c.read_files(sys.argv[1], # File with ambiguous biltrans output
			 sys.argv[2]) # File with disambiguated biltrans output

for sl in sl_tl:
	newtl = sorted(sl_tl[sl], key=lambda x: sl_tl[sl][x])
	newtl.reverse()
	first = True
	for tl in newtl:
		if first:
			print('%.10f %s %s @' % (sl_tl[sl][tl] , common.wrap(sl) , common.wrap(tl)))
			first = False
		else:
			print('%.10f %s %s' % (sl_tl[sl][tl] , common.wrap(sl) , common.wrap(tl)))
