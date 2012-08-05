#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;

inf = open(sys.argv[1]);
count = 1;
for line in inf.readlines(): #{
	if count == 2: #{
		row = line.strip().split('\t');
		outl = row[1];
		outl = outl.replace(' ', '$ ^');
		outl = '^' + outl + '$';
		outl = outl.replace('~', ' ');

		print(row[0] + '\t' + outl);
	#}
	if line.count('-----------------------------') > 0: #{
		count = 0;
	#}
	count = count + 1;
#}
