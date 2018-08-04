#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, re;

# Input:
#        a) Frequency lexicon
#        b) Biltrans output
#        c) Disambiguated biltrans output
#	 d) Crispiness threshold

MAX_NGRAMS = 3;

cur_line = 0;
crisphold = 3.0 ;
rsep = re.compile('\$[^\^]*\^');
if len(sys.argv) == 5: #{
	crisphold = float(sys.argv[4]);
	print('crisp:', crisphold, file=sys.stderr);
#}

sl_tl_defaults = {};
sl_tl = {};
ngrams = {};

for line in open(sys.argv[1]).readlines(): #{
	if len(line) < 1: #{
		continue;
	#}
	row = line.split(' ');
	sl = row[1];
	tl = row[2];
	if line.count('@') > 0: #{
		sl_tl_defaults[sl] = tl;
	else: #{
		sl_tl[sl] = tl;
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

	if len(rsep.findall(am_line)) != len(rsep.findall(dm_line)): #{
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
		#}
	#}
#}

for sl in ngrams: #{

	for ngram in ngrams[sl]: #{
		total = 0;
		max_freq = -1;
		current_tl = '';
		for tl in ngrams[sl][ngram]: #{
			if ngrams[sl][ngram][tl] > max_freq: #{
				max_freq = ngrams[sl][ngram][tl];
				current_tl = tl;
			#}
			total = total + ngrams[sl][ngram][tl];
		#}

		#> If for each of the rules we include
		#> the amount of time the translation is seen with that pattern over the
		#> total, we get a number we can try as a threshold. e.g. > 0.6 >0.7 >0.8
		#> etc.  (>0.6 would be the same as 2/3 of the time the alternative
		#> translation is seen with that ngram, and 1/3 of the time the default
		#> translation is). I think this would be easier to explain than the magic
		#> number I came up with.
		#
		#I see this as a way to define how "crispy" the decisions are. I think it
		#would be better to express this as a ratio: the ratio of the times the
		#alternative translation is seen to the number of times the defaullt
		#translation is seen with that n-gram.
		#
		#It would be "2" in this case: the alternative is seen twice as often as
		#the default.

		for tl in ngrams[sl][ngram]: #{
			crispiness = 0.0;
			default = sl_tl_defaults[sl];
			alt_crisp = float(ngrams[sl][ngram][tl]) / float(total);
			def_crisp = 1.0;
			if default in ngrams[sl][ngram]: #{
				def_crisp = float(ngrams[sl][ngram][default] / float(total));
			#}
			weight = float(ngrams[sl][ngram][tl]) / float(total);
			crispiness = alt_crisp/def_crisp;

			#print '%%%' , crispiness , alt_crisp , def_crisp , tl , default , ngrams[sl][ngram] ;

			if crispiness < crisphold: #{
				print('-', crispiness , weight , total, max_freq, ngrams[sl][ngram][tl], '\t'+ sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][tl]));
			else: #{

				print('+', crispiness , weight , total, max_freq, ngrams[sl][ngram][tl], '\t' +  sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][current_tl]));
			#}
		#}
	#}
#}
