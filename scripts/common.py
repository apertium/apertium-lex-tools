#!/bin/python

import re

re_start = re.compile('(^[^\^]*)');

def tokenize_biltrans_line(line):
	line = clean_biltrans_line(line)[1:-1];
	row = [];
	token = '';
	state = 0;

	escaped = False;

	for c in line:
		# in token
		if state == 0:
			if c == '$':
				row.append(token);
				token = '';
				state = 1;
			elif c == '\\':
				continue;
			else:
				token += c;
		# between tokens
		elif state == 1:

			if c == '\\':
				escaped = True;
			elif c == '^' and not escaped:
				state = 0;
				escaped = False;
			elif escaped:
				escaped = False

	return row

def clean_biltrans_line(line):
	line = re_start.sub('', line);
	return line
