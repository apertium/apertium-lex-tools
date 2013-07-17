#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs;
import common;

if len(sys.argv) < 2: #{
	print('extact-sentences.py <phrasetable> <biltrans>');
	sys.exit(-1);
#}

phrase_table = open(sys.argv[1]);
biltrans_out = open(sys.argv[2]);

def bttoken_tostr(token):
	return '^' + token['sl'] + '/' + '/'.join(token['tls']) + '$';

def generate_tags(token):
	tags = filter(lambda x: x != "*>", token.split('<')[1:]);
	tags = ["<s n=\"" + x.rstrip('>') + "\"/>" for x in tags ];
	tags = ''.join(tags);
	return tags

def generate_entry(slw, tlw):
	out = '<e><p><l>%s%s</l><r>%s%s</r></p></e>';
	llemma = slw.split('<')[0]
	ltags = generate_tags(slw);

	rlemma = tlw.split('<')[0]
	rtags = generate_tags(tlw);

#	ltags = ["<s n=\"" + x + "\"/>" for x in ltags]

	print (out % (llemma, ltags, rlemma, rtags));


def ambiguous(bt): #{
	# legislation<n><sg>/legislación<n><f><sg>/ordenamiento<n><m><sg>
	
	ambig = False;
	for token in bt: #{
		tls = token['tls']
		if len(tls) > 1: #{
			return True;
		#}
	#}

	return ambig;
#}

reading = True;
lineno = 0;
total_valid = 0;

while reading: #{	
	try:
		lineno = lineno + 1;
		pt_line = phrase_table.readline().strip();	
		bt_line = biltrans_out.readline().strip();

		if bt_line == '' and pt_line == '': #{
			reading = False;
		#}

		row = pt_line.split('|||');
		sl = common.tokenize_tagger_line(row[0]);
		tl = common.tokenize_tagger_line(row[1]);
		alignments = row[2].strip();
		bt = common.tokenize_biltrans_line(bt_line);

		if not ambiguous(bt): #{
			continue;
		#}
		if len(sl) < 2 and len(tl) < 2: #{
			continue;
		#}

		# Here we collect a set of SL words, with their correspondences in the bilingual
		# dictionary, and the word they have been aligned with in the target.
			# e.g.  words[0] = ('sl', ['bt1', 'bt2', ...], 'tl')

		translations = {};	
		i = 0;
		for j in alignments.split(' '): #{
			ament = j.split('-');
			if int(ament[1]) > len(tl): #{
				continue;
			#}
			slw = sl[int(ament[0])]
			translations[slw] = {}
			translations[slw]['tls'] = tl[int(ament[1])]
			translations[slw]['bts'] = bt[int(ament[0])]
		#}

	#	for tr in translations:
	#		print (tr, translations[tr])

		current_ambig_words = {};
		valid = True;
		i = 0;
		#
		for tran in translations: #{
			r = translations[tran]
			tlw = r['tls']
			# If the word is ambiguous
			if len(r['bts']['tls']) > 1: #{
				# Check to see if the TL possibilities are found in the lexical 
				# transfer output.
				if tlw not in r['bts']['tls']:
					print (tlw, "not found for", tran, file=sys.stderr);
					generate_entry(tran, tlw);
			
		#}

	#}

	except:
		pass
