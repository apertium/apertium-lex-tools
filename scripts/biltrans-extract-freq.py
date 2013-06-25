#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;
import common;

# Input:
#        a) Biltrans output
#        b) Disambiguated biltrans output  

#

sl_tl = {}; # The sl-tl possible combinations
am_file = open(sys.argv[1]); # File with ambiguous biltrans output
dm_file = open(sys.argv[2]); # File with disambiguated biltrans output
reading = True;

while reading: #{
	am_line = am_file.readline();
	dm_line = dm_file.readline();

	if am_line == '' and dm_line == '': #{
		reading = False;
		continue;
	#}

	am_row = common.tokenize_biltrans_line(am_line);
	dm_row = common.tokenize_biltrans_line(dm_line);

	if len(am_row) != len(dm_row): #{
		print('Mismatch in number of LUs between analysis and training', file=sys.stderr);
		print('\t' + am_line, file=sys.stderr);
		print('\t' + dm_line, file=sys.stderr);
		print('...skipping', file=sys.stderr);
		continue;
	#}

	limit = len(am_row);
	for i in range(0, limit): #{
		if am_row[i].count('/') > 1: #{
			#print(am_row[i] , dm_row[i]); 
			sl = am_row[i].split('/')[0].replace(' ', '~');
			tl = dm_row[i].split('/')[1].replace(' ', '~');
			if sl.count('><') > 0: #{
				sl = sl.split('><')[0] + '>';
			#}
			if tl.count('><') > 0: #{
				tl = tl.split('><')[0] + '>';
			#}
			if sl not in sl_tl: #{
				sl_tl[sl] = {};
			#}
			if tl not in sl_tl[sl]: #{
				sl_tl[sl][tl] = 0;
			#}
			sl_tl[sl][tl] = sl_tl[sl][tl] + 1;
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

