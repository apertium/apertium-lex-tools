#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, re;
import common;

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
cache_counts = open('/tmp/cache_counts.log', 'w+');

if len(sys.argv) == 5: #{
	crisphold = float(sys.argv[4]);
	print('crisp:', crisphold, file=sys.stderr);
#}

sl_tl_defaults = {}; 
sl_tl = {};
ngrams = {};

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

rsep = re.compile('\$[^\^]*\^');

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

	current_am_line_id += 1

#	# to skip lines in the frac corpus if we have a sub-corpus
#	if current_dm_line_id != current_am_line_id: #{
#		print('line_id_mismatch: %d != %d' % (current_am_line_id, current_dm_line_id), file=sys.stderr);
#		while current_dm_line_id != current_am_line_id: #{
#			dm_line = dm_file.readline();
#			current_dm_line_id = int(dm_line.split('.[][')[1].split(' ')[0]);       
#			print('skipping %d ...' % (current_dm_line_id), file=sys.stderr);
#		#}
#	#}
	while current_dm_line_id == current_am_line_id: #{

		am_row = common.tokenize_biltrans_line(am_line);
		dm_row = common.tokenize_biltrans_line(dm_line);

		if len(am_row) != len(dm_row): #{
			print('Mismatch in number of LUs between analysis and training', file=sys.stderr);
			print('\t' + am_line, file=sys.stderr);
			print('\t' + dm_line, file=sys.stderr);
			print('...skipping', file=sys.stderr);
			continue;
		#}
	
		cur_sl_row = [];
		for lu in am_row: #{
			sl = lu.split('/')[0];
			if sl.count('><') > 0: #{
				sl = sl.split('><')[0] + '>';
			#}
			cur_sl_row.append(sl);
		#}

		try:
			frac_count = float(dm_line.split('\t')[2]);
		except:
			break;
	
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
	
#				if tl !=  sl_tl_defaults[sl]: #{
#					print('+' , sl , sl_tl_defaults[sl] , tl, file=sys.stderr);
#				else: #{
#					print('-' , sl , sl_tl_defaults[sl] , tl, file=sys.stderr);
#				#}
	
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

	if am_counter % 10000 == 0: #{
		print('=> %d SL and %d TL lines [id: %d] [ngrams: %d].' % (am_counter, dm_counter, current_am_line_id, len(ngrams)), file=sys.stderr);
		sys.stderr.flush();
	#}
#}

print('Caching counts...', file=sys.stderr);
for sl in ngrams: #{

	for ngram in ngrams[sl]: #{

		for tl in ngrams[sl][ngram]: #{
			print('%.10f\t%s\t%s\t%s' % (ngrams[sl][ngram][tl], ngram, sl, tl), file=cache_counts);		
		#}
	#}
#}
print('\n', file=sys.stderr);

for sl in ngrams: #{

	for ngram in ngrams[sl]: #{
		try:
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

			total = 0.0;
			max_freq = 0.0;
			max_tl = '';
			for tl in ngrams[sl][ngram]: #{
				if ngrams[sl][ngram][tl] > max_freq: #{
					max_freq = ngrams[sl][ngram][tl];
					max_tl = tl;
				#}
				total = total + ngrams[sl][ngram][tl];
			#}

			if only_max == True: #{
				crispiness = 0.0;
				default = sl_tl_defaults[sl];	
	#			if default == max_tl: #{
	#				print('default=max_tl', default, max_tl, '\t', ngram, file=sys.stderr);
	#			else:#{
	#				print('default!=max_tl', default, max_tl, '\t', ngram, file=sys.stderr);
	#			#}
				alt_crisp = float(ngrams[sl][ngram][max_tl]) / float(total);
				def_crisp = 1.0;
				if default in ngrams[sl][ngram]: #{
					def_crisp = float(ngrams[sl][ngram][default] / float(total));
				#}
				weight = float(ngrams[sl][ngram][max_tl]) / float(total);
				crispiness = alt_crisp/def_crisp;

				if crispiness < crisphold: #{
					print('- %.10f %.10f %.10f %.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (crispiness, weight, total, ngrams[sl][ngram][default] , max_freq, ngrams[sl][ngram][max_tl], sl, ngram, max_tl, ngrams[sl][ngram][max_tl]));
	#				print('-', crispiness , weight , total, ngrams[sl][ngram][default] , max_freq, ngrams[sl][ngram][max_tl], '\t'+ sl + '\t' + ngram + '\t' + max_tl + '\t' + str(ngrams[sl][ngram][max_tl]));
				else: #{

					print('+ %.10f %.10f %.10f %.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (crispiness, weight, total, ngrams[sl][ngram][default] , max_freq, ngrams[sl][ngram][max_tl], sl, ngram, max_tl, ngrams[sl][ngram][max_tl]));
					#print('+', crispiness , weight , total, ngrams[sl][ngram][default] , max_freq, ngrams[sl][ngram][max_tl], '\t' +  sl + '\t' + ngram + '\t' + max_tl + '\t' + str(ngrams[sl][ngram][max_tl]));
				#}

	#   crispiness   weight      total default     max_freq     tl_freq            sl
	#+ 2.61845457309 0.7236389238 1.0 0.2763610762 0.7236389238 0.7236389238         aozer<n>        aozer<n> an<det> levr<n>        organisateur<n> 0.7236389238
	#- 14736.0468727 0.9999321438 1.0 0.9999321438 0.9999321438      treuzkas<n>     treuzkas<n> teknologel<adj>     transfert<n>    0.9999321438


	
			else: #{
		
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
						print('- %.10f %.10f %.10f %.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (crispiness, weight, total, ngrams[sl][ngram][default] , max_freq, ngrams[sl][ngram][tl], sl, ngram, tl, ngrams[sl][ngram][tl]));
					else: #{
						print('+ %.10f %.10f %.10f %.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (crispiness, weight, total, ngrams[sl][ngram][default] , max_freq, ngrams[sl][ngram][tl], sl, ngram, tl, ngrams[sl][ngram][tl]));
					#}
	#+ 1013.01568891 0.9989973752 2.0 1.9979947504 1.9979947504 	galloud<n>	ha<cnjcoo> an<det> galloud<n>	puissance<n>	1.9979947504

				#}
			#}
		except:
			pass
	#}
#}
