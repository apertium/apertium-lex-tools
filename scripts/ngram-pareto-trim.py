#!/bin/python

import sys
from collections import defaultdict

ngrams = defaultdict(lambda: defaultdict(dict))
translations = defaultdict(set)

def make_line(ngrams, sl, ngram, tl):
	return '+ ' + str(ngrams[sl][ngram][tl]) + '\t' + sl + '\t' + ngram + '\t' + tl + '\t1'

def compose_single_translation(ngrams, sl, pareto):
	s = 0
	for ngram in ngrams[sl]:
		for tl in ngrams[sl][ngram]:
			if tl in pareto:
				s += ngrams[sl][ngram][tl]

	return '+ ' + str(s) + '\t' + sl + '\t' + '-' + '\t' + list(pareto)[0] + '\t1'

def dominates(xs, ys):
	fst = 1
	snd = 1
	for x, y in zip(xs, ys):
		if x > y: snd = 0
		if y > x: fst = 0

	if fst > snd:
		return 1
	elif snd > fst:
		return -1
	else:
		return 0

def calculate_pareto_frontier(sl, ngrams, translations):
	pareto = set()
	m = []
	for tl in translations:
		v = []
		for ngram in ngrams:
			if tl in ngrams[ngram]:
				v.append(ngrams[ngram][tl])
			else:
				v.append(0)
		m.append((tl, v))

	for x in m:
		if all(map(lambda y: dominates(x[1], y[1]) >= 0, m)):
			pareto.add(x[0])

	return pareto


for line in sys.stdin:
	row = line.rstrip().split('\t')
	weight, sl, ngram, tl = row[:4]
	weight = float(weight.split()[1])

	translations[sl].add(tl)

	if tl not in ngrams[sl][ngram]:
		ngrams[sl][ngram][tl] = weight

for sl in ngrams:
	pareto = calculate_pareto_frontier(sl, ngrams[sl], translations[sl])
	if len(pareto) == 1:
		print(sl, 'has a single translation:', list(pareto)[0], file=sys.stderr)
		print(compose_single_translation(ngrams, sl, pareto))
		continue

	for ngram in ngrams[sl]:
		for tl in ngrams[sl][ngram]:
			if tl in pareto:
				print (make_line(ngrams, sl, ngram, tl))
