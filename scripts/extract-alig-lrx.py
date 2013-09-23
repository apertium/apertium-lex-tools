#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys;
import common

with open(sys.argv[1]) as d:
	print('<rules>');
	for line in d: #{

		sys.stdout.flush();
		if line[-2] == '@': #{
			row = common.tokenize_tagger_line(line)

			fq = line.split(' ')[0];
			sl = row[0];
			tl = row[1];

			if line.count('>') < 2: #{
				continue;
			#}
			print(sl, tl, file=sys.stderr)
			sl_lem = sl.split('<')[0];
			tl_lem = tl.split('<')[0];
			sl_lem = sl_lem.replace('-', '\\-').replace('~', ' ').replace('&', '&amp;');
			tl_lem = tl_lem.replace('-', '\\-').replace('~', ' ').replace('&', '&amp;');

			sl_tag = sl.replace('><', '.').split('<')[1].strip('>');
			tl_tag = tl.replace('><', '.').split('<')[1].strip('>');

			cmb = '';
			cma = '';

			if sl_tag.split('.')[0] not in ['adj', 'vblex', 'n']: #{
				cmb = '<!--';
				cma = '-->';	
			else: #{
				cma = '';
				cmb = '';
			#}

			rule = cmb + '<rule comment="' + fq + '">';
			#rule = rule + '<match lemma="' + sl_lem + '" tags="' + sl_tag + '"><select lemma="' + tl_lem + '" tags="' + tl_tag + '"/>';	
			rule = rule + '<match lemma="' + sl_lem + '"><select lemma="' + tl_lem + '"/>';	
			rule = rule + '</match>';
			rule = rule + '</rule>' + cma;
	
			print(rule);
		#}
	

	#}
	print('</rules>');
