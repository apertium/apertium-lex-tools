#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

# This script takes a lexically ambiguous bilingual dictionary and produces an LRX file
# which selects the 'default' translations for each word.
#
# It supports two kinds of ambiguous bilingual dictionaries, the kind with 'slr' entries
# for example mk-en, br-fr. And the kind without 'slr' entries, such as eu-es. 
#
# In the case of the 'noslr' type, it reads the whole dictionary, keeping all the left
# sides in a hash, with their corresponding right sides in a list, marked with the
# direction restriction. It then iterates through the list, generating a rule when 
# there is a left side with >1 translation possibility, where all the other translation
# possibilities are marked with a direction restriction.
#
# In the case of the 'slr' type, it reads the dictionary line by line, if it finds a line
# with 'c="0"' then it marks this line as possibly ambiguous, and then if it finds a following
# line with slr="1" then it generates a rule to select the line with c="0".
#
# Usage:
#  python3 extract-default-lrx.py apertium-eu-es.eu-es.dix lr  > outfile
#


import sys;

def sideToTagList(s): #{
	tags = '<'.join(s.split('<')[1:]);
	
	tags = tags.replace('"/><s n="', '.');
	tags = tags.replace('<s n="', '');
	tags = tags.replace('s n="', '');
	tags = tags.replace('"/>', '');
	tags = tags + '.*';		
	return tags;
#}

d = open(sys.argv[1]);
sentit = sys.argv[2];

poss_ambig = 0;
sl = '';
default = '';
tipus = 'noslr';

sl_tl = {};

for line in d.readlines(): #{
	if line.count('slr=') > 0: #{
		tipus = 'slr';
	#}
	if line.count('<l>') > 0 and line.count('<r>') > 0: #{
		r = '';
		sl = '';
		tl = '';
		if sentit == 'lr': #{
			sl = line.split('<l>')[1].split('</l>')[0];
			tl = line.split('<r>')[1].split('</r>')[0];
		elif sentit == 'rl': #{
			sl = line.split('<r>')[1].split('</r>')[0];
			tl = line.split('<l>')[1].split('</l>')[0];
		#}
		if line.count('r="LR"'): #{
			r = 'LR';
		elif line.count('r="RL"'): #{
			r = 'RL';
		#}

		if sl not in sl_tl: #{
			sl_tl[sl] = [];
		#}
		sl_tl[sl].append((r, tl));
	#}
#}

d.close();
d = open(sys.argv[1]);

print('<rules>');

if tipus == 'noslr': #{

	for key in sl_tl.keys(): #{
		if len(sl_tl[key]) > 1: #{
			#print(key, sl_tl[key]);
			num_def = 0;
			default = '';
			for tl in sl_tl[key]: #{
				if sentit == 'lr': #{
					if tl[0] == '' or tl[0] == 'LR': #{
						default = tl;
						num_def = num_def + 1 ;	
					#}	
				elif sentit == 'rl': #{
					if tl[0] == '' or tl[0] == 'RL': #{
						default = tl;
						num_def = num_def + 1 ;	
					#}	
				#}
			#}	
			if num_def == 1: #{
				rule = '';
				sl_lem = key;
				sl_lem = sl_lem.replace('<b/>', ' ');
				sl_lem = sl_lem.replace('</g>', '');
				sl_lem = sl_lem.replace('<g>', '#');
				sl_lem = sl_lem.replace('-', '\\-');
				sl_tag = sideToTagList(sl_lem);
				sl_lem = sl_lem.split('<')[0];
				tl_lem = default[1];
				tl_lem = tl_lem.replace('<b/>', ' ');
				tl_lem = tl_lem.replace('</g>', '');
				tl_lem = tl_lem.replace('<g>', '#');
				tl_lem = tl_lem.replace('-', '\\-');
				tl_tag = sideToTagList(tl_lem);
				tl_lem = tl_lem.split('<')[0];
				rule = '  <rule>'; 
				rule = rule + '<match lemma="' + sl_lem + '" tags="' + sl_tag + '"><select lemma="' + tl_lem + '" tags="' + tl_tag + '"/></match>';
				rule = rule + '</rule>';
				print(rule);

				#print(key, default[1]);	
			elif num_def < 1: #{
				print('WARNING: No default translations of ' + key + '.', file=sys.stderr);
				print('\t', sl_tl[key], file=sys.stderr);
			elif num_def > 1: #{
				print('WARNING:', num_def, 'default translations of ' + key + '.', file=sys.stderr);
				print('\t', sl_tl[key], file=sys.stderr);
			#}
		#}
	#}	

else:
	for line in d.readlines(): #{
		line = line.strip();
	
		if line.count('c="') > 0 and line.count('0"') > 0 and poss_ambig == 1: #{
			poss_ambig = 0;	
		#}
		if line.count('c="') > 0 and line.count('0"') > 0 and poss_ambig == 0: #{
			poss_ambig = 1;	
			if sentit == 'lr': #{
				sl = line.split('<l>')[1].split('</l>')[0];
				default = line.split('<r>')[1].split('</r>')[0];
			elif sentit == 'rl': #{
				sl = line.split('<r>')[1].split('</r>')[0];
				default = line.split('<l>')[1].split('</l>')[0];
			#}
		#}
		
		if line.count('slr="') > 0 and line.count('1"') > 0 and poss_ambig == 1: #{
			if sentit == 'lr': #{
				nsl = line.split('<l>')[1].split('</l>')[0];
			elif sentit == 'rl': #{
				nsl = line.split('<r>')[1].split('</r>')[0];
			#}
	
			if nsl == sl: #{
				#print(sl , default);
				sl_lem = sl;
				sl_lem = sl_lem.replace('<b/>', ' ');
				sl_lem = sl_lem.replace('</g>', '');
				sl_lem = sl_lem.replace('<g>', '#');
				sl_lem = sl_lem.replace('-', '\\-');
				sl_tag = sideToTagList(sl_lem);
				sl_lem = sl_lem.split('<')[0];
				tl_lem = default;
				tl_lem = tl_lem.replace('<b/>', ' ');
				tl_lem = tl_lem.replace('</g>', '');
				tl_lem = tl_lem.replace('<g>', '#');
				tl_lem = tl_lem.replace('-', '\\-');
				tl_tag = sideToTagList(tl_lem);
				tl_lem = tl_lem.split('<')[0];
				rule = '  <rule>'; 
				rule = rule + '<match lemma="' + sl_lem + '" tags="' + sl_tag + '"><select lemma="' + tl_lem + '" tags="' + tl_tag + '"/></match>';
				rule = rule + '</rule>';
				print(rule);
			#}
		#}
	#}
#}	

print('</rules>');
