#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys

def processWord(c):
	lemma = ''
	tags = ''
	c = sys.stdin.read(1)
	while c != '<':
		if c == '*':
			while c and c != '$':
				lemma += c
				c = sys.stdin.read(1)
			sys.stdout.write(lemma.replace(' ', '~') + ' ')
			return
		lemma += c
		c = sys.stdin.read(1)
	while c != '$':
		tags += c
		c = sys.stdin.read(1)
	sys.stdout.write(lemma.replace(' ', '~') + tags.replace(' ', '~') + ' ')

c = sys.stdin.read(1)
while c:
	# Beginning of a lexical unit
	if c == '^':
		processWord(c)
	if c == '[':
		while c and c != ']':
			if c == '\n':
				sys.stdout.write('\n')
			c = sys.stdin.read(1)
	# Newline is newline
	if c == '\n':
		sys.stdout.write('\n')
	c = sys.stdin.read(1)
