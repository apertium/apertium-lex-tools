#!/bin/python

import sys
import regex
import re
import collections

def clean_line(line):
	line = re.compile(' ').sub('<br/>', line.strip());
	line = re.compile('\$[^\^]*\^').sub('$~^', line);
	line = re.compile('^[^\^]+').sub('', line);
	line = re.compile('\$[^$\$]+$').sub('', line);
	return re.compile('~').split(line);

def process_bidix(bidix, open_tag, close_tag):
	dix = {}

	tag_regex = re.compile('"([^"/]+)');
	left_stem_regex = re.compile('%s([^<]+)<' % open_tag)
	left_entry_regex = re.compile('%s[^<]+<(?:(?!%s).)+' % (open_tag, close_tag), re.U)
	left_entries = left_entry_regex.findall(bidix);

	for entry in left_entries:
		stem = left_stem_regex.findall(entry)[0]
		tags = tag_regex.findall(entry);
		if stem not in dix:
			dix[stem] = []
		dix[stem].append(tags)

	return dix
		

def find_in_bidix(token, bidix, stem_regex, tag_regex):

	try:
		if(token[1] == '*'):
			return token;
		else:
			stem = stem_regex.findall(token)[0];
			token_tags = tag_regex.findall(token);
			if(token_tags[0] != 'np'):
				stem = stem.lower();

			if stem not in bidix:
				return token

			max_len = 0;
			max_tags = [];
			entries = bidix[stem]
			for bidix_tags in entries:
				bidix_tags_multiset = collections.Counter(bidix_tags);
				token_tags_multiset = collections.Counter(token_tags);
		
				l = len(list((token_tags_multiset & bidix_tags_multiset).elements()));
				if l > max_len:
					max_len = l
					max_tags = bidix_tags

			return '^' + stem + ''.join(map(lambda x: '<' + x + '>', max_tags)) + '$';
	except:
		return token

if __name__ == "__main__":

	if len(sys.argv) != 3:
		print sys.argv[1] + ' <bidix_source_file> <sl|tl>'

	side = sys.argv[2];
	if side == 'sl':
		open_tag = '<l>'
		close_tag = '</l>'
	elif side == 'tl':
		open_tag = '<r>'
		close_tag = '</r>'
	else:
		print "tl or sl"
		sys.exit(1);

	tag_regex = re.compile('<([^/>]+)>');
	stem_regex = re.compile('\^([^<]+)');

	bidix = open(sys.argv[1], 'r').read();
	bidix_bin = process_bidix(bidix, open_tag, close_tag);

	for line in sys.stdin.readlines():
		tokens = clean_line(line);
		out = ''
		for token in tokens:
			out += find_in_bidix(token, bidix_bin, stem_regex, tag_regex) + ' ';
		print out
		
