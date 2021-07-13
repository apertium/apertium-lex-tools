#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
from operator import mul

t = 0
lineno = 0

def parse_input(line):
	sentence = []
	escaped = False
	reading_word = False
	lu = ''

	for c in line:
		if escaped:
			if reading_word:
				lu += c
		elif c == '\\':
			if reading_word:
				lu += c
			escaped = True
		elif c == '^':
			reading_word = True
		elif c == '$':
			sentence.append(lu.split('/')[1:])
			reading_word = False
			lu = ''
		elif reading_word:
			lu += c

	return sentence

def process(sentence, start, out):
	global t
	global lineno
	if start >= len(sentence):
		sen = ' '.join('^'+s+'$' for s in out)
		print('.[][%d %d].[]\t%s' % (lineno, t, sen))
		t += 1
		return

	for token in sentence[start]:
		process(sentence, start + 1, out + [token])

while True:
	string = sys.stdin.readline().rstrip()
	if string == "":
		break

	tokens = parse_input(string)
	# print map(len, tokens)
	# print reduce(mul, map(len, tokens))

	process(tokens, 0, [])
	lineno += 1
	t = 0
