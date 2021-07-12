#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import biltrans_count_common as BCC
from collections import defaultdict

# Input:
#        a) Biltrans output
#        b) biltrans output
#	 c)
#
#11 	if<cnjadv>/si<cnjadv> the<det><def><sp>/el<det><def><GD><ND> house<n><sg>/casa<n><f><sg> agree<vblex><pri><p3><sg>/acordar<vblex><pri><p3><sg>/concordar<vblex><pri><p3><sg>/estar#~de~acuerdo<vblex><pri><p3><sg> ,<cm>/,<cm> prpers<prn><subj><p1><mf><sg>/prpers<prn><tn><p1><mf><sg> shall<vaux><inf>/ do<vblex><inf>/hacer<vblex><inf> as<preadv>/tan<preadv> Mr~Evans<np><ant><mf><sg>/pn000Evans<np><ant><mf><sg> have<vbhaver><pri><p3><sg>/haber<vbhaver><pri><p3><sg> suggest<vblex><pp>/sugerir<vblex><pp> .<sent>/.<sent>


#11 	si<cnjadv> el<det><def> asamblea<n> estar#~de~acuerdo<vblex> ,<cm> hacer<vblex> lo~que<rel><nn> el<det><def> señor<n> Evans<np><cog> acabar<vblex> de<pr> sugerir<vblex><inf> .<sent>

# The sl-tl possible combinations
sl_tl = defaultdict(lambda: defaultdict(lambda: 0))

class Counter(BCC.BiltransCounter):
	tokenizer = 'biltrans'
	line_ids = False

	def process_row(self, frac_count=0):
		global sl_tl
		for i in range(len(self.am_row)):
			if self.am_row[i].count('/') > 1:
				sl = BCC.strip_tags(am_row[i], 'sl', space=True)

				bts = am_row[i].split('/')[1:]
				valid_trads = set(BCC.strip_tags(b, 'sl', space=True)
								  for b in bts)

				for tl_ in dm_row:
					tl = BCC.strip_tags(tl_, 'sl', space=True)
					if tl in valid_trads:
						sl_tl[sl][tl] += 1

c = Counter()
c.read_files(sys.argv[1], # File with ambiguous biltrans output
			 sys.argv[2]) # File with biltrans output

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
