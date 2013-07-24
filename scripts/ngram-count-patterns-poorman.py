#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, math, re, common;

# Input:

#        a) Biltrans output

# 1562	^na<pr><loc>/на<pr>$ ^zagrebački<adj><*>/загребски<adj><*>$ ^*summitu/*summitu$ ^naglasiti<vblex><perf><tv><*>/нагласи<vblex><perf><tv><*>$ ^regionalan<adj><*>/регионален<adj><*>$ ^energetski<adj><*>/енергетски<adj><*>$ ^suradnja<n><f><*>/соработка<n><f><*>$


#        b) Tagger output

# 1562	^Самит<n><m><*>$ ^во<pr>$ ^Загреб<np><*>$ ^*фрли$ ^*светлина$ ^на<pr>$ ^регионален<adj><*>$ ^енергетски<adj><*>$ ^соработка<n><f><*>$

MAX_NGRAMS = 3; # Max = 5-grams

cur_line = 0;

def found_tls(tls, dm_row):
	found = []
	for tl in tls:
		if tl in dm_row:
			found.append(tl)

	return found

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print ("Usage:", sys.argv[0], "<biltrans output>", "<tagger output>");
		sys.exit(1);

	ngrams = {};
	print('Reading...', file=sys.stderr);
	sys.stderr.flush();

	am_file = open(sys.argv[1]); # File with ambiguous biltrans output
	dm_file = open(sys.argv[2]); # File with tagger output
	reading = True;
	lineno = 0;
	while reading: #{
		lineno += 1
		if (lineno % 1000 == 0):
			print ("at line no: ", lineno, file=sys.stderr);
		am_line = am_file.readline();
		dm_line = dm_file.readline();
		if am_line == '': #{
			reading = False;
			continue;
		#}
		try:
			am_row = common.tokenize_biltrans_line(am_line);
			dm_row = set(common.tokenize_tagger_line(dm_line));
		except:
			continue;

		cur_sl_row = [x['sl'] for x in am_row];

		for i in range(0, len(am_row)): #{
			sl = am_row[i]['sl'];
			tls = am_row[i]['tls'];
			if len(tls) < 2: #{
				continue;
			#}


			for tl in found_tls(tls, dm_row): #{
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
					ngrams[sl][pregram][tl] += 1
					ngrams[sl][postgram][tl] += 1
					ngrams[sl][roundgram][tl] += 1
				#}

			#}
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
