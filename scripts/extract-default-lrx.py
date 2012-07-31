#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

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
print('<rules>');
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
print('</rules>');
