#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

if len(sys.argv) < 2: #{
	print 'extact-sentences.py <phrasetable> <biltrans>';
	sys.exit(-1);
#}


phrase_table = file(sys.argv[1]);
biltrans_out = file(sys.argv[2]);

def ambiguous(bt): #{
	# ^legislation<n><sg>/legislación<n><f><sg>/ordenamiento<n><m><sg>$
	
	ambig = False;
	for w in bt.split(' '): #{
		if w.count('/') > 1: #{
			if w.count('<n>') > 0 or w.count('<adj>') > 0 or w.count('<vblex>') > 0: #{
				return True;
			#}
		#}
	#}

	return ambig;
#}

reading = True;
lineno = 0;

while reading: #{
	lineno = lineno + 1;
	pt_line = phrase_table.readline().decode('utf-8');	
	bt_line = biltrans_out.readline().decode('utf-8');

	if bt_line == '' and pt_line == '': #{
		reading = False;
	#}

        # Shitty tokenisation
	bt_line = bt_line.replace(u'$^', '$ ^'); 
	bt_line = bt_line.replace(u'^', ' ^');

	if not ambiguous(bt_line): #{
		continue;
	#}

	bt_line = bt_line.strip('[] ').strip().replace('  ', ' ').replace(' " ', ' ').replace(u'ěř ', ' ').replace(u'č ', ' ').replace(u'ě ', ' ').replace('" ', ' ').replace(u' – ', ' ').replace(u'š ', ' ').replace(u'ý ', ' ').replace(u'ů ', ' ').replace(u' Š ', ' ').replace(u'ř ', ' ').replace(u'Š ', ' ').replace(u' ‘ ', ' ').replace(u' Ř ', ' ').replace(u'ž ', ' ').replace(u'\/ ', ' ').replace(u' Č ', ' ').replace(u'— ', ' ').replace(u'… ', ' ').replace(u' & ', ' ').replace('+ ', ' ').replace('  ', ' ');

	#print lineno , pt_line ; 
	#print lineno , bt_line ; 

	row = pt_line.split('|||');
	sl = row[1].strip();
	tl = row[0].strip();
	aliniaments = row[2].strip();

	bt_row = bt_line.split('$ ^');
	sl_row = sl.split(' ');
	tl_row = tl.split(' ');

	if len(sl_row) < 2 and len(tl_row) < 2: #{
		continue;
	#}

	if len(sl_row) != len(bt_row): #{
		print len(sl_row) , '!=', len(bt_row);
	#}

	# check that len(sl) = len(bt)
        # words[0] = ('sl', ['bt1', 'bt2', ...], 'tl')
	# for each of the source words
	#   
        # 

	words = {};
	i = 0;
	for i in range(0, len(sl_row)): #{
		tl_vals = [];
		#print lineno , i , sl_row[i] , bt_row[i] 
		#print lineno , i , aliniaments;
		for j in aliniaments.split(' '): #{
			ament = j.split('-');
			if int(ament[1]) == i: #{
				t_ament = int(ament[0]);
				if t_ament > len(tl_row): #{
					continue;
				#}
				tl_vals.append(tl_row[t_ament]);
			#}
		#}
		#print bt_row[i];
		bt_vals = bt_row[i].split('/')[1:];
		words[i] = (sl_row[i], bt_vals, tl_vals);
	#}

	#print '-------------------------------------------------------------------------------';

	current_ambig_words = {};

	valid = True;
	i = 0;
	for word in words: #{
		#print lineno , i ,  words[word];
		if len(words[word][1]) > 1: #{
			current_ambig_words[i] = words[word];
			# ambig: (u'spend<vblex><pp>', [u'pasar<vblex><pp>', u'llevar<vblex><pp>', u'gastar<vblex><pp>', u'perder<vblex><pp>', u'emplear<vblex><pp>', u'dedicar<vblex><pp>'], [u'gastar<vblex><inf>'])

			#print >>sys.stderr, 'ambig:' , words[word];
			for tlw in words[word][2]: #{
				found = False;
				for btw in words[word][1]: #{
					if btw.count(tlw.split('<')[0] + '<') > 0: #{
						found = True;
					#}
				#}
				if not found: #{
					#print >>sys.stderr, 'MISSING:' , words[word];
					valid = False;
				#}
			#}
		#}
		i = i + 1;
	#}
	
	if not valid: #{
		continue;
	#}

	print lineno, '\t' + sl;
	print lineno, '\t' + bt_line;
	print lineno, '\t' + tl;
	print lineno, '\t' + aliniaments;

	# Resumption<n> of<pr> the<def><def> session<n> 
	# Resumption<n><sg>/Reanudación<n><f><sg> of<pr>/de<pr> the<det><def><sp>/el<det><def><GD><ND> session<n><sg>/sesión<n><f><sg> 
	# Reanudación<n> de<pr> el<det><def> periodo<n> de<pr> sesión<n> 
	# 0-0 1-1 2-2 5-3

	print '-------------------------------------------------------------------------------';
	
#}

