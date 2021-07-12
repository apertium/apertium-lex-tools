#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, re
import common
import biltrans_count_common as BCC

# Input:
#        a) Frequency lexicon
#        b) Biltrans output
#        c) Disambiguated biltrans output
#	 d) Crispiness threshold

cur_line = 0
crisphold = 3.0
if len(sys.argv) == 5:
	crisphold = float(sys.argv[4])
	print('crisp:', crisphold, file=sys.stderr)

sl_tl, sl_tl_defaults, _ = BCC.read_frequencies(sys.argv[1])

class Counter(BCC.BiltransCounter):
	tokenizer = 'regex'
	line_ids = False
	count_ngrams = True
	max_ngrams = 3

	def process_lu(self, sl, tl, idx, cur_sl_row, frac_count=0):
		global sl_tl_defaults
		sym = '-' if tl == sl_tl_defaults[sl] else '+'
		print(sym, sl, sl_tl_defaults[sl], tl, file=sys.stderr)


c = Counter()
c.read_files(sys.argv[2], # File with ambiguous biltrans output
			 sys.argv[3]) # File with disambiguated biltrans output
ngrams = c.ngrams

for sl in ngrams:
	for ngram in ngrams[sl]:
		total = 0
		max_freq = -1
		current_tl = ''
		for tl in ngrams[sl][ngram]:
			if ngrams[sl][ngram][tl] > max_freq:
				max_freq = ngrams[sl][ngram][tl]
				current_tl = tl
			total += ngrams[sl][ngram][tl]

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

		for tl in ngrams[sl][ngram]:
			crispiness = 0.0
			default = sl_tl_defaults[sl]
			alt_crisp = float(ngrams[sl][ngram][tl]) / float(total)
			def_crisp = 1.0
			if default in ngrams[sl][ngram]:
				def_crisp = float(ngrams[sl][ngram][default] / float(total))

			weight = float(ngrams[sl][ngram][tl]) / float(total)
			crispiness = alt_crisp/def_crisp

			#print '%%%' , crispiness , alt_crisp , def_crisp , tl , default , ngrams[sl][ngram]

			if crispiness < crisphold:
				print('-', crispiness , weight , total, max_freq, ngrams[sl][ngram][tl], '\t'+ sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][tl]))
			else:
				print('+', crispiness , weight , total, max_freq, ngrams[sl][ngram][tl], '\t' +  sl + '\t' + ngram + '\t' + tl + '\t' + str(ngrams[sl][ngram][current_tl]))
