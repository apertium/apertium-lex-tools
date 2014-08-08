#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs;
import common;

if len(sys.argv) < 3 or len(sys.argv) > 4: #{
	print('extact-sentences.py <phrasetable> <biltrans> [-m|--match-pos]');
	sys.exit(-1);
#}

match_pos = False;
if len(sys.argv) == 4 and sys.argv[3] not in ['-m', '--match-pos']:
	print('extact-sentences.py <phrasetable> <biltrans> [-m|--match-pos]');
	sys.exit(-1);
elif len(sys.argv) == 4 and sys.argv[3] in ['-m', '--match-pos']:
	match_pos = True;

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

def pos_equal(s, t):
	spos = s.split('>')[1][1:]
	tpos = s.split('>')[1][1:]

	return spos == tpos;
	

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
		sl = common.tokenise_tagger_line(row[1]);
		tl = common.tokenise_tagger_line(row[0]);
		alignments = row[2].strip();
		bt = common.tokenise_biltrans_line(bt_line);

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
			if int(ament[0]) > len(tl): #{
				continue;
			#}
			slw = sl[int(ament[1])]
			if slw not in translations:
				translations[slw] = {}
			translations[slw]['tls'] = tl[int(ament[0])]
			translations[slw]['bts'] = bt[int(ament[1])]
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
				# if match_pos = 1 and pos tags do not match
				if match_pos and not pos_equal(tran, tlw):
					continue;

				# Check to see if the TL possibilities are found in the lexical 
				# transfer output.
				if tlw not in r['bts']['tls']:
					print (tlw, "not found for", tran, file=sys.stderr);
					generate_entry(tran, tlw);
			
		#}

	#}

	except:
		pass
