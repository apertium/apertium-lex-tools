#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys;
import common;

#+nature<n>	service<n> nature<n>	carácter<n>	3
#+nature<n>	The<def><def> imperialist<adj> nature<n>	carácter<n>	1
#+nature<n>	the<def><def> secular<adj> nature<n> of<pr> State<n>	carácter<n>	1
#+nature<n>	its<det><pos> nature<n> prevent<vblex>	carácter<n>	1
#+nature<n>	nature<n> be<vbser> in<pr>	carácter<n>	1
#

#FREQMIN = 8.0;

MINMATCH = 2;

infile = '';

if len(sys.argv) < 2: #{
	print('ngrams-to-rules.py <ngrams>');
	sys.exit(-1);
#}

infile = open(sys.argv[1]);
	

permitted_tags = ['n', 'vblex', 'adj'];

print('<rules>');
lineno = 1;
ruleno = 0;
for line in infile: #{
#	print '\n';
#	print line
	if len(line) < 2: #{
		continue;
	#}
	line = line.strip();
	#line = line.decode('utf-8').strip();
	print(line, file=sys.stderr)
	#+ 0.571428571429 14 8 8 	troiñ<vblex>		tourner<vblex>	8
	row = line.split('\t');

	tipus = row[0].split(' ')[0];
	weight = row[0].replace('  ', ' ').split(' ')[1];
	sl = row[1].strip()[1:-1];
	tl = row[3][1:-1];
	tl_lema = tl.split('<')[0].lower();
	tl_tags = ''.join(tl.split('<')[1:]).replace('>', '.').rstrip('.')
	freq = 1
#	freq = float(row[4]);

	pattern = common.tokenize_tagger_line(row[2]);

	if row[2].count('<guio>') > 0 or row[2].count('<sent>') > 0 or row[2].count('<cm>') > 0: #{
		print('PUNCTUATION_IN_PATTERN', line, file=sys.stderr);
		continue;
	#}

	if tipus == '-' or tipus == '~': #{
		print('DEFAULT_READING', line, file=sys.stderr);
		continue;
	#}

	# Hacks
#	if len(pattern) == 0: #{
#		print('ZERO_PATTERN' , line, file=sys.stderr);
#		continue;
	#}


	if len(pattern) < MINMATCH and len(pattern) > 0: #{
		print('BELOW_MINMATCH', line, file=sys.stderr);
		continue;
	#}

	

	inpattern = False;
	for w in pattern: #{
		if w.lower().count(sl) > 0: #{
			inpattern = True;
		#}
	#}
	if len(pattern) > 0 and not inpattern:  #{
		print('SL_NOT_IN_PATTERN' , line, file=sys.stderr);
		continue;
	#}

	if tl_tags.count('adj') > 0 and sl.count('adj')  < 1: #{
		print("TAG_MISMATCH" , line, file=sys.stderr);
		continue;
	#}
	if tl_tags.count('vbmod') > 0 and sl.count('vbmod')  < 1: #{
		print("TAG_MISMATCH" , line, file=sys.stderr);
		continue;
	#}

	if tl_tags.split('.')[0] not in permitted_tags: #{
		print("TAG_NOT_PERMITTED" , tl_tags , '||' , line, file=sys.stderr);
		continue;
	#}

	sel = False;
	ruleno = ruleno + 1;
	lineno = lineno + 1;

	commentb = '';
	commente = '';
#	if freq < FREQMIN: #{
#		commentb = '<!--';
#		commente = '-->';
#	#}
	
	print(commentb + '  <rule c="' + str(ruleno) + ' ' + str(lineno) + ': ' + str(freq) + '" weight="' + weight + '">');
	for word in pattern: #{
		sl_lema = word.split('<')[0].lower();
		if word.count('><') > 0: #{
			sl_tags = '<'.join(word.split('<')[1:]).replace('><', '.').replace('>', '');
		else: #{
			sl_tags = '<'.join(word.split('<')[1:]).strip('<>');
		#}

		# ======================================================================= #

		sl_lema = sl_lema.replace('~', ' ');
		tl_lema = tl_lema.replace('~', ' ');
#		sl_lema = sl_lema.replace('-', '\-');
#		tl_lema = tl_lema.replace('-', '\-');
#		sl_lema = sl_lema.replace('(', '\(');
#		tl_lema = tl_lema.replace('(', '\(');
#		sl_lema = sl_lema.replace(')', '\)');
#		tl_lema = tl_lema.replace(')', '\)');
#
		if word.lower().count(sl) > 0: #{
			lineno = lineno + 1;
			if sl_lema == '': #{
				print('    <match tags="' + sl_tags + '"><select lemma="' + tl_lema + '" tags="' + tl_tags + '"/></match>');
			else: #{
				print('    <match lemma="' + sl_lema + '" tags="' + sl_tags + '"><select lemma="' + tl_lema + '" tags="' + tl_tags + '"/></match>');
			#}
			sel = True;
		else: #{
			lineno = lineno + 1;
			if sl_lema == '': #{	
				print('    <match tags="' + sl_tags + '"/>');
			else: #{
				print('    <match lemma="' + sl_lema + '" tags="' + sl_tags + '"/>');
			#}
		#}
	#}
	if sel == False and len(pattern) == 0: #{
		sl_lema = sl.split('<')[0];
		if sl.count('><') > 0: #{
			sl_tags = '<'.join(sl.split('<')[1:]).replace('><', '.').replace('>', '');
		else: #{
			sl_tags = '<'.join(sl.split('<')[1:]).strip('<>');
		#} 
		if sl_lema == '': #{
			print('    <match tags="' + sl_tags + '"><select lemma="' + tl_lema + '" tags="' + tl_tags + '"/></match>');
		else: #{
			print('    <match lemma="' + sl_lema + '" tags="' + sl_tags + '"><select lemma="' + tl_lema + '" tags="' + tl_tags + '"/></match>');
		print('  </rule>' + commente);
	elif sel == False:
		print('  </rule>'+commente+ '<!-- Warning: No select operation ', line, '-->');
	else: #{
		print('  </rule>' + commente);
	#}
	lineno = lineno + 1;
#}
print('</rules>');
