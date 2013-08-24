#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, math, re, common;

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

MAX_NGRAMS = 3; # Max = 5-grams

cur_line = 0;
crisphold = 3.0 ; # Default
only_max = True;
#only_max = False;

if len(sys.argv) == 5: #{
	crisphold = float(sys.argv[4]);
	print('crisp:', crisphold, file=sys.stderr);
#}

sl_tl_defaults = {}; 
sl_tl = {};
ngrams = {};

def clean_line(l): #{
	newline = '';
	inword = True;
	for c in l: #{
		if c == '\t': 
			newline = newline + c; 
			inword = False; 
			continue;
		if c == '^': 
			newline = newline + c; 
			inword = True; 
			continue;
		if c == '$': 
			newline = newline + c + ' ';  
			inword = False;
			continue;
		if inword == True: #{
			newline = newline + c;
		#}
	#}
#	print(newline, file=sys.stderr);
	return newline;
#}


# First read in the frequency defaults

for line in open(sys.argv[1]).readlines(): #{
	if len(line) < 1: #{
		continue;
	#}
	row = line.split(' ');
	sl = row[1];
	tl = row[2];
	fr = float(row[0]);
	if line.count('@') and fr == 0.0: #{
		print('!!! Prolly something went wrong here, the default has a freq of 0.0', file=sys.stderr);
		print('    %s => %s = %.10f' % (sl, tl, fr), file=sys.stderr);
	#}
	if line.count('@') > 0: #{
		print('default:', sl, tl, file=sys.stderr);
		sl_tl_defaults[sl] = tl;
	else: #{
		sl_tl[sl] = tl;
	#}
	
#}

print('Reading...', file=sys.stderr);
sys.stderr.flush();

am_file = open(sys.argv[2]); # File with ambiguous biltrans output
dm_file = open(sys.argv[3]); # File with disambiguated biltrans output
reading = True;

current_am_line_id = -1;
current_dm_line_id = -1;

dm_line = dm_file.readline();
current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);

am_counter = 0;
dm_counter = 0;

while reading: #{
	am_line = am_file.readline();

	if am_line == '': #{
		reading = False;
		continue;
	#}
	current_am_line_id = int(am_line.split("\t")[0]);

#	# to skip lines in the frac corpus if we have a sub-corpus
	if current_dm_line_id != current_am_line_id: #{
		print('line_id_mismatch: %d != %d' % (current_am_line_id, current_dm_line_id), file=sys.stderr);
#		while current_dm_line_id != current_am_line_id: #{
#			dm_line = dm_file.readline();
#			current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);       
#			print('skipping %d ...' % (current_dm_line_id), file=sys.stderr);
#		#}
	#}
	while current_dm_line_id == current_am_line_id: #{

		am_row = common.tokenize_biltrans_line(am_line);
		dm_row = common.tokenize_biltrans_line(dm_line);

		if len(am_row) != len(dm_row): #{
			amc = len(am_row);
			dmc = len(dm_row);
			print('Mismatch in number of LUs between analysis and training', file=sys.stderr);
			print('am(',amc,'):\t' + am_line, file=sys.stderr);
			print('dm(',dmc,'):\t' + dm_line, file=sys.stderr);
			print('...skipping', file=sys.stderr);
			dm_line = dm_file.readline();
			if dm_line == '': #{
				reading = False;
				break;
			#}
			current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);
			dm_counter += 1;

			continue;
		#}


		frac_count = 0.0;
		s_fc = dm_line.split('\t')[2].strip();
		if s_fc == '' or len(s_fc) == 0: #{
			print('%d %d :: %d %d :: Frac count is not floatable' % (am_counter, dm_counter, current_am_line_id, current_dm_line_id), file=sys.stderr);		
		#}
		try:
			frac_count = float(s_fc);
		except:
			break
		if math.isnan(frac_count): #{ 
			print('%d %d :: %d %d :: Frac count is not a number' % (am_counter, dm_counter, current_am_line_id, current_dm_line_id), file=sys.stderr);		
			frac_count = 0.0;
		#}

		limit = len(am_row);
		cur_sl_row = [x['sl'] for x in am_row];

		for i in range(0, limit): #{
			if len(am_row[i]['tls']) > 1: #{
				
				sl = am_row[i]['sl']
				tl = dm_row[i]['tls'][0]

				for j in range(1, MAX_NGRAMS): #{
					pregram = ' '.join(map(common.wrap, cur_sl_row[i-j:i+1]));
					postgram = ' '.join(map(common.wrap, cur_sl_row[i:i+j+1]));
					roundgram = ' '.join(map(common.wrap, cur_sl_row[i-j:i+j+1]));
	
					if sl not in ngrams: #{
						ngrams[sl] = {};
					#}
					if pregram not in ngrams[sl]: #{
						ngrams[sl][pregram] = {};
					#}
					if postgram not in ngrams[sl]: #{
						ngrams[sl][postgram] = {};
					#}
					if roundgram not in ngrams[sl]: #{
						ngrams[sl][roundgram] = {};
					#}
					if tl not in ngrams[sl][pregram]: #{
						ngrams[sl][pregram][tl] = 0.0;
					#}
					if tl not in ngrams[sl][postgram]: #{
						ngrams[sl][postgram][tl] = 0.0;
					#}
					if tl not in ngrams[sl][roundgram]: #{
						ngrams[sl][roundgram][tl] = 0.0;
					#}
					ngrams[sl][pregram][tl] = ngrams[sl][pregram][tl] + frac_count;
					ngrams[sl][postgram][tl] = ngrams[sl][postgram][tl] + frac_count;
					ngrams[sl][roundgram][tl] = ngrams[sl][roundgram][tl] + frac_count;
	
#					print('=> %s\t[%.10f] %s' % (tl, ngrams[sl][pregram][tl], pregram), file=sys.stderr);
#					print('=> %s\t[%.10f] %s' % (tl, ngrams[sl][roundgram][tl], roundgram), file=sys.stderr);
#					print('=> %s\t[%.10f] %s' % (tl, ngrams[sl][postgram][tl], postgram), file=sys.stderr);
	

				#}
			#}
		#}
		
		dm_line = dm_file.readline();
		if dm_line == '': #{
			reading = False;
			break;
		#}
		current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);

		dm_counter += 1;
	#}
	am_counter += 1;

	if am_counter % 1000 == 0: #{
		print('=> %d SL and %d TL lines [id: %d] [ngrams: %d].' % (am_counter, dm_counter, current_am_line_id, len(ngrams)), file=sys.stderr);
		sys.stderr.flush();
	#}
#}

print('Caching counts...', file=sys.stderr);
for sl in ngrams: #{

	for ngram in ngrams[sl]: #{

		for tl in ngrams[sl][ngram]: #{
			print('%.10f\t%s\t%s\t%s' % (ngrams[sl][ngram][tl], ngram, sl, tl));		
		#}
	#}
#}
print('\n', file=sys.stderr);
