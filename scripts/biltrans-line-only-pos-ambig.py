#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys

pos = ["<n>", "<vblex>", "<adj>"]

def process_line(l):
	global pos
	w = ''
	in_word = False
	escaped = False
	for c in l:
		if c == '\\':
			escaped = True
			continue
		if c == '^' and escaped == False:
			in_word = True
		if c == '$' and escaped == False:
			if w.count('/') > 1 and any(p in w for p in pos):
				return True
			w = ''
			in_word = False
		if in_word == True:
			w += c
		escaped = False
	return False

output = False

infile = sys.stdin

if len(sys.argv) > 1:
	infile = open(sys.argv[1])

for line in infile.readlines():
	output = process_line(line)
	if output == True:
		print(line.strip())
#	else:
#		print(line.strip(), file=sys.stderr)
