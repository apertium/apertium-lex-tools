#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys

def process_biltrans_unit(lu, sents):
	new_paths = {}

	state = 0
	tl = {}
	ls = lu[1:-1].split('/')
	sl = ls[0]
	for i in range(1, len(ls)):
		tl[i] = '/' + ls[i]

	if len(tl) == 0:
		print('ERROR:', lu, file=sys.stderr)
	elif len(tl) > 1:
		for tid, trad in tl.items():
			for path, sent in sents.items():
				new_paths[path + trad] = sent + '^' + sl + trad + '$'
	else:
		for path in sents:
			new_paths[path] = sents[path] + '^' + sl + tl[1] + '$'
	return new_paths

def process_line(line):
	escaped = False
	in_word = False
	cur_id = line.split()[0]
	idx = len(cur_id) + 1
	lu = ''
	output_sentences = {'':''}
	while idx < len(line):
		c = line[idx]
		if c == '\\':
			if in_word:
				lu += c
				idx += 1
				lu += line[idx]
			else:
				idx += 1
		elif c == '^':
			in_word = True
		elif c == '$':
			in_word = False
			new_paths = process_biltrans_line(lu, output_sentences)
			output_sentences = new_paths
			lu = ''
		elif in_word:
			lu += c
		elif c.isspace() and c != '\n':
			for s in output_sentences:
				output_sentences[s] += c
		idx += 1
	return cur_id, output_sentences

while True:
	ln = sys.stdin.readline()
	if not ln:
		break
	cur_id, sentences = process_line(ln)
	print('output_sentences:', len(sentences), file=sys.stderr)
	for i, sent in enumerate(sentences.values()):
		print('.[][%s %s].[]\t%s' % (cur_id, i, sent))
