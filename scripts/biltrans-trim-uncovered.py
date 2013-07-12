#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys;

pos = ["<n>", "<vblex>", "<adj>"];

output = False;	

infile = sys.stdin ; 

if len(sys.argv) > 1: #{
	infile = open(sys.argv[1]);
#}

lineno = 0;
for line in infile.readlines(): #{
	lineno = lineno + 1;
	num_lu = float(line.count('$'));
	num_unk = float(line.count('*'))  / 2.0;
	cov = 100.0 - ((num_unk / num_lu) * 100.0);
	
	if cov >= 90.0: #{
		print(line.strip());
	#} 
	
	
#}
