#!/bin/python

import re

re_start = re.compile('(^[^\^]*)');

def parse_tags(ptr, line):
	tags = []
	tag = '';
	
	while True:
		c = line[ptr];
		if c == '$' or c == '/':
			return (ptr-1, tags);
		elif c == '>':
			tags.append(tag);
			tag = '';
		elif c != '<':
			tag += c;

		ptr += 1

def parse_sl(ptr, line):
	out = '';
	if line[ptr] == '*':
		(ptr, out) = parse_unknown(ptr, line)
		return (ptr, (out, []));

	escaped = False;
	while True:
		c = line[ptr];
		if c == '\\':
			escaped = True;
		elif c == '/' and not escaped:
			return (ptr, out);
		elif c == '<' and not escaped:
			(ptr, tags) = parse_tags(ptr+1, line);
			return (ptr, (out, tags));
		else:
			out += c;
			escaped = False;
		ptr += 1;

def parse_unknown(ptr, line):
	out = '';
	escaped = False;
	while True:
		c = line[ptr];
		if c == '\\':
			escaped = True;
		elif (c == '$' or c == '/') and not escaped:
			return (ptr, out);
		else:
			out += c;
			escaped = False;
		ptr += 1;

def parse_tls(ptr, line):
	tls = [];
	tl = '';
	out = '';
	escaped = False;
	if line[ptr] == '*':
		(ptr, out) = parse_unknown(ptr, line)
		return (ptr, [(out, [])]);


	while True:
		if ptr == len(line):
			tls.append(tl)
			return (ptr, tls);
		c = line[ptr];
		if c == '\\':
			escaped = True;
		elif c == '/' and tl != '' and not escaped:
			tls.append(tl)
			tl = '';
		elif c == '$' and not escaped:
			if tl != '':
				tls.append(tl)
			return (ptr, tls);
		elif c == '<' and not escaped:
			(ptr, tags) = parse_tags(ptr, line);
			tls.append((tl, tags));
			tl = '';
		elif c != '/' or escaped:
			tl += c;
			escaped = False;
		ptr += 1;


def trim_tags(sl, tls):
	new_tls = []
	new_sl = (sl[0], [])

	tagslist = [set(sl[1])]
	for tl in tls:
		tagslist.append(set(tl[1]));
	tag_intersection = set.intersection(*tagslist)

	for tl in tls:
		new_tl = (tl[0], []);
		if len(tl[1]) > 0:
			new_tl[1].append(tl[1][0]);
			
		for i in range(1, len(tl[1])):
			if tl[1][i] not in tag_intersection:
				new_tl[1].append(tl[1][i])
		
		new_tl = new_tl[0] + '<' + '><'.join(new_tl[1]) + '>';
		new_tls.append(new_tl);

	flag_all = True;
	if len(sl[1]) > 0:
		new_sl[1].append(sl[1][0]);
	for i in range(1, len(sl[1])):
		tag = sl[1][i];
		if tag not in tag_intersection:
			new_sl[1].append(tag)
			
	new_sl = new_sl[0] + '<' + '><'.join(new_sl[1]) + '>';

	return (new_sl, new_tls);
		

def parse_token(ptr, line):
	(ptr, sl) = parse_sl(ptr, line);
	(ptr, tls) = parse_tls(ptr+1, line);
	(sl, tls) = trim_tags(sl, tls);
	
	return (ptr, sl, tls);
		

def tokenize_biltrans_line(line):

	out = []
	escaped = False;
	for ptr in range(0, len(line)):
		c = line[ptr];
		if c == '^' and not escaped:
			(ptr, sl, tls) = parse_token(ptr+1, line)
			out.append((sl, tls));
		elif c == '\\':
			escaped = True;
		elif escaped:
			escaped = False;

	return out

def tokenize_biltrans_line2(line):
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
