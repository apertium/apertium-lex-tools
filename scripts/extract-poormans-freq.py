#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;
import common
# Input:
#        a) Biltrans output
#        b) biltrans output  
#	 c)
#
#11 	if<cnjadv>/si<cnjadv> the<det><def><sp>/el<det><def><GD><ND> house<n><sg>/casa<n><f><sg> agree<vblex><pri><p3><sg>/acordar<vblex><pri><p3><sg>/concordar<vblex><pri><p3><sg>/estar#~de~acuerdo<vblex><pri><p3><sg> ,<cm>/,<cm> prpers<prn><subj><p1><mf><sg>/prpers<prn><tn><p1><mf><sg> shall<vaux><inf>/ do<vblex><inf>/hacer<vblex><inf> as<preadv>/tan<preadv> Mr~Evans<np><ant><mf><sg>/pn000Evans<np><ant><mf><sg> have<vbhaver><pri><p3><sg>/haber<vbhaver><pri><p3><sg> suggest<vblex><pp>/sugerir<vblex><pp> .<sent>/.<sent>


#11 	si<cnjadv> el<det><def> asamblea<n> estar#~de~acuerdo<vblex> ,<cm> hacer<vblex> lo~que<rel><nn> el<det><def> señor<n> Evans<np><cog> acabar<vblex> de<pr> sugerir<vblex><inf> .<sent>


sl_tl = {}; # The sl-tl possible combinations
am_file = open(sys.argv[1]); # File with ambiguous biltrans output
dm_file = open(sys.argv[2]); # File with biltrans output
reading = True;

while reading: #{
	am_line = am_file.readline();
	dm_line = dm_file.readline();

	if am_line == '' and dm_line == '': #{
		reading = False;
		continue;
	#}
	try:
		am_row = common.tokenize_biltrans_line(am_line);
		dm_row = common.tokenize_tagger_line(dm_line);
	except:
		continue;


	limit = len(am_row);
	for i in range(0, limit): #{
		if len(am_row[i]['tls']) > 1: #{
			sl = am_row[i]['sl'];
			if sl not in sl_tl: #{
				sl_tl[sl] = {};
			#}
			bts = am_row[i]['tls'];
			valid_trads = set();
			for bt in bts: #{
				valid_trads.add(bt);
			#}
			limit2 = len(dm_row);
			for j in range(0, limit2): #{
				tl = dm_row[j];
				if tl not in valid_trads: #{
					continue;
				#}
				if tl not in sl_tl[sl]: #{
					sl_tl[sl][tl] = 0;
				#}
				sl_tl[sl][tl] = sl_tl[sl][tl] + 1;
			#}
		#}
	#}	
#}

for sl in sl_tl: #{
	newtl = sorted(sl_tl[sl], key=lambda x: sl_tl[sl][x])
	newtl.reverse()
	first = True;
	for tl in newtl: #{
		if first: #{
			print(sl_tl[sl][tl] , sl , tl , '@');
			first = False
		else: #{
			print(sl_tl[sl][tl] , sl , tl);
		#}
	#}
#}

