#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs;
import common;

if len(sys.argv) < 2: #{
	print('extact-sentences.py <phrasetable> <biltrans>');
	sys.exit(-1);
#}

phrase_table = open(sys.argv[1]);
biltrans_out = open(sys.argv[2]);

def ambiguous(bt): #{
	# legislation<n><sg>/legislación<n><f><sg>/ordenamiento<n><m><sg>
	
	ambig = False;
	for token in bt: #{
		tls = token['tls']
		if len(tls) > 1: #{
			return True;
		#}
	#}

	return ambig;
#}

reading = True;
lineno = 0;
total_valid = 0;

while reading: #{	
	try:
		lineno = lineno + 1;
		pt_line = phrase_table.readline().strip();	
		bt_line = biltrans_out.readline().strip();

		if not bt_line.strip() and not pt_line.strip(): #{
			reading = False;
			break
		#}
		elif not bt_line.strip() or not pt_line.strip():
			continue;

		row = pt_line.split(' ||| ');
		bt = common.tokenize_biltrans_line(bt_line);
		sl = common.tokenize_tagger_line(row[1]);
		tl = common.tokenize_tagger_line(row[0]);

		
		if not ambiguous(bt): #{
			print ("line", lineno, "not ambiguous", file=sys.stderr);
			continue;
		#}
		if len(sl) < 2 and len(tl) < 2: #{
			continue;
		#}


		# Check that the number of words in the lexical transfer, and in the phrasetable matches up
		if len(sl) != len(bt): #{
			print ("len(sl) != len(bt)", file=sys.stderr);
			continue;
		#}


		# Resumption<n> of<pr> the<def><def> session<n> 
		# Resumption<n><sg>/Reanudación<n><f><sg> of<pr>/de<pr> the<det><def><sp>/el<det><def><GD><ND> session<n><sg>/sesión<n><f><sg> 
		# Reanudación<n> de<pr> el<det><def> periodo<n> de<pr> sesión<n> 
		# 0-0 1-1 2-2 5-3


		print(lineno, '\t' + row[1]);
		print(lineno, '\t' + bt_line);
		print(lineno, '\t' + row[0]);
		print(lineno, '\t' + row[2]);
		print('-------------------------------------------------------------------------------');
		total_valid += 1
	except:
		continue

#}

print('total:', lineno, file=sys.stderr);
print('valid:', total_valid, '(' + str((total_valid/lineno)*100) + '%)', file=sys.stderr);

