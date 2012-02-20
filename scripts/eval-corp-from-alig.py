#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

if len(sys.argv) < 2: #{
	print 'extact-sentences.py <alignments> <biltrans> <target>';
	sys.exit(-1);
#}

alignments = file(sys.argv[1]);
biltrans_out = file(sys.argv[2]);
target_tagged = file(sys.argv[3]);

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

def line_to_row(bt): #{
	words = [];
	inWord = False;
	word = '';
	for c in bt: #{
		if c == '^': #{
			inWord = True;		
			continue;
		#}	
		if c == '\\': #{
			escaped = True;
			continue;
		#}
		if c == '$' and escaped == False: #{
			words.append(word);
			word = '';
			inWord = False;
		#}
		if inWord: #{
			word = word + c;
		#}
		escaped = False;
	#}
	return words;
#}

reading = True;
lineno = 0;

while reading: #{
	lineno = lineno + 1;
	al_line = alignments.readline().decode('utf-8');	
	bt_line = biltrans_out.readline().decode('utf-8');
	tg_line = target_tagged.readline().decode('utf-8');

	if bt_line == '' and al_line == '' and tg_line == '': #{
		reading = False;
	#}
	
	al_num = al_line.split('\t')[0];
	al_line = al_line.split('\t')[1];
        # Shitty tokenisation
	bt_line = bt_line.replace(u'$^', '$ ^'); 

	bt_line = bt_line.replace(u'^', ' ^');

#	if not ambiguous(bt_line): #{
#		continue;
#	#}

	bt_row = line_to_row(bt_line);
	tg_row = line_to_row(tg_line);

	print lineno, al_line;
	print lineno, bt_line;
	print lineno, tg_line;
	print lineno, len(tg_row) , tg_row;
	print lineno, len(bt_row) , bt_row;
	print '----';

#2 0-1 0-2 3-0 4-3 5-4 6-5
#     0                                                             1                                           2                               3      
#2 ] ^Nature<n><sg>/Carácter<n><f><sg>/Naturaleza<n><f><sg>$  ^protection<n><sg>/protección<n><f><sg>$  ^officer<n><pl>/agente<n><mf><pl>$  ^accuse<vblex><past>/acusar<vblex><past>$  ^of<pr>/de<pr>$  ^blackmail<n><sg>/chantaje<n><m><sg>$[
#     0                    1         2                     3                     4                        5           6
#2 ]^Defensor<n><m><sg>$ ^de<pr>$ ^el<det><def><f><sg>$ ^naturaleza<n><f><sg>$ ^acusar<vblex><pp><m><sg>$ ^de<pr>$ ^chantaje<n><m><sg>$[
	
	i = 0;
	sys.stdout.write(al_num + ' ' + str(lineno) + ' dab: ');
	for word in bt_row: #{
		resolved = [];
		if word.count('/') > 1: #{
			for alig in al_line.split(' '): #{
				# 0-3 2-0 3-2 4-1 5-4 6-7 7-5 8-6 9-8 11-9 12-15 13-16 15-10 16-11 18-12 20-13 21-14 22-17
				l = int(alig.split('-')[0]);
				r = int(alig.split('-')[1]);
				if r == i: #{
					#print 'x' , word , tg_row[l];

					for lu in word.split('/')[1:]: #{
						st_lem = lu.split('<')[0].lower();
						t_lem = tg_row[l].split('<')[0].lower();
						#print st_lem , t_lem

						if st_lem == t_lem: #{
							resolved.append(lu);
						#}
					#}
				#}
			#}		
		#}
		if len(resolved) > 0: #{
			sl = word.split('/')[0];
			out = '^' + sl + '/';
			for tl in resolved: #{
				out = out + tl + '/';
			#}
			out = out + '$ ';
			sys.stdout.write(out.replace('/$', '$'));
		else: #{
			sys.stdout.write('^' + word + '$ ');
		#}
		i = i + 1;
	#}
	sys.stdout.write('\n');

	print '----';
#}
