#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;

# Input:
#        a) Frequency lexicon
#        b) Biltrans output
#        c) Disambiguated biltrans output  

MAX_NGRAMS = 3;

cur_line = 0;

sl_tl_defaults = {}; 
sl_tl = {};
ngrams = {};

meevents = {}; # events[sl][counter] = [feat, feat, feat];
meoutcomes = {}; # meoutcomes[sl][counter] = tl;
event_counter = 0;

features = {}; # features[(sl, ['a', 'list'], tl)] = 3
feature_counter = 0;

indexes = {};
trad_counter = {};



for line in open(sys.argv[1]).readlines(): #{
	if len(line) < 1: #{
		continue;
	#}
	row = line.split(' ');
	sl = row[1];
	tl = row[2].strip();
	if sl not in trad_counter: #{
		trad_counter[sl] = 0;
	#}
	if line.count('@') > 0: #{
		print(sl, tl, file=sys.stderr);
		sl_tl_defaults[sl] = tl;
		indexes[(sl, tl)] = trad_counter[sl];
		trad_counter[sl] = trad_counter[sl] + 1;
	else: #{
		sl_tl[sl] = tl;
		indexes[(sl, tl)] = trad_counter[sl];
		trad_counter[sl] = trad_counter[sl] + 1;
	#}
#}

am_file = open(sys.argv[2]); # File with ambiguous biltrans output
dm_file = open(sys.argv[3]); # File with disambiguated biltrans output
reading = True;

while reading: #{
	am_line = am_file.readline();
	dm_line = dm_file.readline();

	if am_line == '' and dm_line == '': #{
		reading = False;
		continue;
	#}

	if am_line.count('$ ^') != dm_line.count('$ ^'): #{
		print('Mismatch in number of LUs between analysis and training', file=sys.stderr);
		print('\t' + am_line, file=sys.stderr);
		print('\t' + dm_line, file=sys.stderr);
		print('...skipping', file=sys.stderr);
		continue;
	#}


	am_row = am_line.split('\t')[1].replace('$^', '$ ^')[1:-1].split('$ ^');
	dm_row = dm_line.split('\t')[1].replace('$^', '$ ^')[1:-1].split('$ ^');
	cur_sl_row = [];
	for lu in am_row: #{
		sl = lu.split('/')[0];
		if sl.count('><') > 0: #{
			sl = sl.split('><')[0] + '>';
		#}
		cur_sl_row.append(sl);
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

			if tl !=  sl_tl_defaults[sl]: #{
				print('+' , sl , sl_tl_defaults[sl] , tl, file=sys.stderr);
			else: #{
				print('-' , sl , sl_tl_defaults[sl] , tl, file=sys.stderr);
			#}

			for j in range(1, MAX_NGRAMS): #{
				pregram = ' '.join(cur_sl_row[i-j:i+1]);
				postgram = ' '.join(cur_sl_row[i:i+j+1]);
				roundgram = ' '.join(cur_sl_row[i-j:i+j+1]);

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
					ngrams[sl][pregram][tl] = 0;
				#}
				if tl not in ngrams[sl][postgram]: #{
					ngrams[sl][postgram][tl] = 0;
				#}
				if tl not in ngrams[sl][roundgram]: #{
					ngrams[sl][roundgram][tl] = 0;
				#}

				ngrams[sl][pregram][tl] = ngrams[sl][pregram][tl] + 1;
				ngrams[sl][postgram][tl] = ngrams[sl][postgram][tl] + 1;
				ngrams[sl][roundgram][tl] = ngrams[sl][roundgram][tl] + 1;
			#}
			if sl not in meevents: #{
				meevents[sl] = {};
			#}
			if sl not in meoutcomes: #{
				meoutcomes[sl] = {};
			#}
			if event_counter not in meevents: #{
				meevents[sl][event_counter] = [];
			#}
			if event_counter not in meoutcomes: #{
				meoutcomes[sl][event_counter] = '';
			#}
			for ni in ngrams[sl]: #{
				if ni not in features: #{
					feature_counter = feature_counter + 1;
					features[ni] = feature_counter;
				#}
				meevents[sl][event_counter].append(features[ni]);
				#meevents[sl][event_counter].append(feat);
				meoutcomes[sl][event_counter] = tl;
				
			#}
			del ngrams;
			ngrams = {};
			if sl not in sl_tl: #{
				continue;
			#}
			if len(sl_tl[sl]) < 2: #{
				continue;
			#}
			for event in meevents[sl]: #{
				outline = str(indexes[(sl, meoutcomes[sl][event])]) + ' # ';
				for j in range(0,  len(sl_tl[sl])): #{
					for feature in meevents[sl][event]: #{
						outline = outline + str(feature) + ':' + str(j) + ' ';
					#}
					outline = outline + ' # '
				#}
				print(sl , '\t', len(sl_tl[sl]),'\t', outline);
			#}
			del meevents;
			del meoutcomes;
			meevents = {};
			meoutcomes = {};
		#}
	#}
	event_counter = event_counter + 1;
#}

for feature in features: #{
	print(features[feature] , '\t' , feature, file=sys.stderr);
#}


