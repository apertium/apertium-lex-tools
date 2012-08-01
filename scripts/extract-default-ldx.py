#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys;

d = open(sys.argv[1]);
sentit = sys.argv[2];

poss_ambig = 0;
sl = '';
default = '';
for line in d.readlines(): #{
	line = line.strip();
	if line.count('bet>') > 0 or line.count('<sec') > 0 or  line.count('ionary>') > 0 or line.count('sdef') > 0 or line.count('ection>') > 0: #{
		print(line);
	#}

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
			if sentit == 'lr': #{
				print('<e><p><l>' + sl + '</l><r>' + default + '</r></p></e>');
			elif sentit == 'rl': #{
				print('<e><p><l>' + default + '</l><r>' + sl + '</r></p></e>');
			#}
		#}
	#}
#}
