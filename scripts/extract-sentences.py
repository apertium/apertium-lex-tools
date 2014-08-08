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
total_errors = 0;

not_ambiguous = [];

while reading: #{	
	try:
		lineno = lineno + 1;
		pt_line = phrase_table.readline().strip();	
		bt_line = biltrans_out.readline().strip();

		if not bt_line.strip() and not pt_line.strip(): #{
			reading = False;
			break
		elif not bt_line.strip() or not pt_line.strip(): #{
			continue;

		#}
		row = pt_line.split('|||');
		bt = common.tokenise_biltrans_line(bt_line.strip());
		sl = common.tokenise_tagger_line(row[1].strip());
		tl = common.tokenise_tagger_line(row[0].strip());
		
		if not ambiguous(bt): #{
			not_ambiguous.append(str(lineno));
			if len(not_ambiguous) >= 10: #{
				print ("not ambiguous:", ' '.join(not_ambiguous), file=sys.stderr);
				not_ambiguous = [];
			#}
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
		print ("error in line", lineno, file=sys.stderr);
		total_errors += 1
		continue

#}

print('total:', lineno, file=sys.stderr);
print('valid:', total_valid, '(' + str((total_valid/lineno)*100) + '%)', file=sys.stderr);
print('errors:',total_errors, '(' + str((total_errors/lineno)*100) + '%)', file=sys.stderr);
