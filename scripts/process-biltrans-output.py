#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

c = sys.stdin.read(1);
 

def processWord(c): #{
	lemma = '';
	tags = '';

	c = sys.stdin.read(1);
	while c != '<': #{
		if c == '*': #{
			while c and c != '$': #{
				lemma = lemma + c;
				c = sys.stdin.read(1);
			#}
			sys.stdout.write(lemma.replace(' ', '~') + ' ');
			return;
		#}
		lemma = lemma + c;
		c = sys.stdin.read(1);
	#}

	while c != '$': #{
		tags = tags + c;
		c = sys.stdin.read(1);
	#}
	
	sys.stdout.write(lemma.replace(' ', '~') + tags.replace(' ', '~') + ' ');
#}

while c: #{
	# Beginning of a lexical unit
	if c == '^': #{
		processWord(c);
	#}

	if c == '[': #{

		while c and c != ']': #{

			if c == '\n': #{
				sys.stdout.write('\n');
			#}
			c = sys.stdin.read(1);
		#}
	#}

	# Newline is newline
	if c == '\n': #{
		sys.stdout.write('\n');
	#}

	c = sys.stdin.read(1);
#}

