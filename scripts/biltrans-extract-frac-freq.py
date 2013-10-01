#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, re
import common
import math

# Input:
#        a) Biltrans output
# 56011   ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$
#
#        b) Disambiguated biltrans output  
#.[][56011 0].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/excuse<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$ 0.9917274061    |@|
#.[][56011 1].[] ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^digarez<n><m><sg>/occasion<n><f><sg>$ ^da<pr>/à<pr>$ ^distreiñ<vblex><inf>/revenir<vblex><inf>$ ^war<pr>/sur<pr>$ ^e<det><pos><m><sp>/son<det><pos><GD><ND>$ ^doare<n><m><sg>/manière<n><f><sg>$ ^ober<vblex><inf>/faire<vblex><inf>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$       0.0082725939    ||
#
#

sl_tl = {}; # The sl-tl possible combinations
am_file = open(sys.argv[1]); # File with ambiguous biltrans output
dm_file = open(sys.argv[2]); # File with disambiguated biltrans output
reading = True;

current_am_line_id = -1;
current_dm_line_id = -1;

rsep = re.compile('\$[^\^]*\^');

dm_line = dm_file.readline();
current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);

while reading: #{
	current_am_line_id += 1
	am_line = am_file.readline();
	if am_line == '':
		reading = False;
		continue;
	#}
	current_am_line_id = int(am_line.split("\t")[0])
	while current_am_line_id == current_dm_line_id: #{
		if current_dm_line_id % 1000 == 0:
			print ("STATUS: at line", current_dm_line_id, file=sys.stderr);
		if dm_line == '': #{
			print('breaking', file=sys.stderr);
			reading = False;
			break;
		#}
		try:
			frac_count = float(dm_line.split('\t')[2]);
			if math.isnan(frac_count):
				frac_count = 0;
		except:
			break;
	
		am_row = common.tokenize_biltrans_line(am_line);
		dm_row = common.tokenize_biltrans_line(dm_line);

		if len(am_row) != len(dm_row): #{
			print('Mismatch in number of LUs between analysis and training', file=sys.stderr);
			print('\t' + am_line, file=sys.stderr);
			print('\t' + dm_line, file=sys.stderr);

			print('\t' + am_line, file=sys.stderr);
			print('\t' + dm_line, file=sys.stderr);
			dm_line = dm_file.readline();
			if dm_line == '': break;
			current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);

		#}

		limit = len(am_row);
		for i in range(0, limit): #{
			if len(am_row[i]['tls']) > 1: #{
			
				sl = am_row[i]['sl']
				tl = dm_row[i]['tls'][0]

				if sl not in sl_tl: #{
					sl_tl[sl] = {};
				#}
				if tl not in sl_tl[sl]: #{
					sl_tl[sl][tl] = 0.0;
				#}
				sl_tl[sl][tl] = sl_tl[sl][tl] + frac_count;
		
			#}
		#}
		dm_line = dm_file.readline();
		if dm_line == '': break;
		current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);

	#}	
#}


for sl in sl_tl: #{
	newtl = sorted(sl_tl[sl], key=lambda x: sl_tl[sl][x])
	newtl.reverse()
	first = True;
	for tl in newtl: #{
		if first: #{
			print('%.10f %s %s @' % (sl_tl[sl][tl] , common.wrap(sl) , common.wrap(tl)));
			first = False
		else: #{
			print('%.10f %s %s' % (sl_tl[sl][tl] , common.wrap(sl) , common.wrap(tl)));
		#}
	#}
#}
