#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys;

d = open(sys.argv[1]);

print('<rules>');
for line in d.readlines(): #{
	if line[-2] == '@': #{
		row = line.split(' ');
	
		fq = row[0];
		sl = row[1];
		tl = row[2];

		sl_lem = sl.split('<')[0];
		tl_lem = tl.split('<')[0];
		sl_lem = sl_lem.replace('-', '\\-').replace('~', ' ');
		tl_lem = tl_lem.replace('-', '\\-').replace('~', ' ');
		sl_tag = sl.replace('><', '.').split('<')[1].strip('>');
		tl_tag = tl.replace('><', '.').split('<')[1].strip('>');
		rule = '<rule comment="' + fq + '">';
		#rule = rule + '<match lemma="' + sl_lem + '" tags="' + sl_tag + '"><select lemma="' + tl_lem + '" tags="' + tl_tag + '"/>';	
		rule = rule + '<match lemma="' + sl_lem + '"><select lemma="' + tl_lem + '"/>';	
		rule = rule + '</match>';
		rule = rule + '</rule>';
		
		print(rule);
	#}

#}
print('</rules>');
