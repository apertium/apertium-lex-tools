#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, random;

# ngram file 
# lambda file
# lexicon file 

ngf = sys.argv[1];
ldf = sys.argv[2];

ngrams = {};
lambdas = {};

for line in open(ngf).readlines(): #{
	#59763 	poor<adj> in<pr> capital<adj>
	if len(line) < 2: #{
		continue;
	#}
	row = line.strip().split('\t');
	ngid = int(row[0].strip());
	ngrams[ngid] = row[1];
#}

for line in open(ldf).readlines(): #{
	#59176:0 1.00131
	if line.count('@@') > 0: #{
		continue;
	#}
	row = line.strip().split(' ');

	l = float(row[1]);
	ngid = int(row[0].split(':')[0]);
	ngram = ngrams[ngid];
	trad = row[0].split(':')[1];

	print(l, '\t', trad, '\t', ngram);
#}
