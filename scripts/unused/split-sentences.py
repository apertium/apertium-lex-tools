#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, random;

def convert_to_biltrans(bt): #{
	outline = '';
	for word in bt.split(' '): #{
		outline = outline + '^' + word.replace('~', ' ') + '$ ';
	#}
	return outline;
#}

# Take the ambiguous lexical transfer output and return 
# an unambiguous lexical transfer output, en la medida de lo posible.
def disambiguate_with_alig(bt, al, tl): #{
	outline = '';
	i = 0;
	tl_row = tl.split(' ');
	for word in bt.split(' '): #{
		resolved = [];
		if word.count('/') > 1: #{
			for alig in al.split(' '): #{
				# 0-3 2-0 3-2 4-1 5-4 6-7 7-5 8-6 9-8 11-9 12-15 13-16 15-10 16-11 18-12 20-13 21-14 22-17
				l = int(alig.split('-')[0]);
				r = int(alig.split('-')[1]);
				if r == i: #{
					#print 'x' , word , tg_row[l];

					for lu in word.split('/')[1:]: #{
						st_lem = lu.split('<')[0].lower();
						t_lem = tl_row[l].split('<')[0].lower();
						#print st_lem , t_lem

						if st_lem == t_lem: #{
							resolved.append(lu);
						#}
					#}
				#}
			#}	      
		#}
		if len(resolved) > 0: #{
			slw = word.split('/')[0];
			out = '^' + slw + '/';
			for tlw in resolved: #{
				out = out + tlw + '/';
			#}
			out = out + '$ ';
			outline = outline + out.replace('/$', '$').replace('~', ' ');
		else: #{
			outline = outline + '^' + word.replace('~', ' ') + '$ ';
		#}
		i = i + 1;
	#}	
	#print(outline);
	return outline;
#}

if len(sys.argv) < 2: #{
	print('split-sentences.py <candidates> <tstlen>');
	sys.exit(-1);
#}

candidates = open(sys.argv[1]);
testlen = int(sys.argv[2]);

# We read through the file and collect all the line numbers.
# Then we randomise and select the top _tstlen_ for the test
# set and the next top _tstlen_ for the dev set.
# The rest are printed out

linenos = [];

for line in candidates.readlines(): #{
	if line.count('--') > 2: #{
		continue;
	#}
	num = line.split('\t');
	linenos.append(int(num[0]));
#}

candidates.close();

linenos = set(linenos);
linenos = list(linenos);

random.shuffle(linenos);

print(len(linenos));

tst = linenos[0:testlen];
dev = linenos[testlen:testlen*2];
train = linenos[2000:];

candidates = open(sys.argv[1]);

trainout = open(sys.argv[1].replace('candidates', 'train'), 'w');
tst_refout = open(sys.argv[1].replace('candidates', 'tst') + '.ref', 'w');
tst_srcout = open(sys.argv[1].replace('candidates', 'tst') + '.src', 'w');
dev_refout = open(sys.argv[1].replace('candidates', 'dev') + '.ref', 'w');
dev_srcout = open(sys.argv[1].replace('candidates', 'dev') + '.src', 'w');

cur_line = -1;
state = 0;
sl = '';
tl = '';
al = '';
bt = '';
for line in candidates.readlines(): #{
	line = line.strip();
	if line.count('--') > 2: #{
		if cur_line in tst: #{
			#print(cur_line, 'tst');
			outline = disambiguate_with_alig(bt, al, tl);	
			print(cur_line, ']\t' + outline, file=tst_refout);
			outline = convert_to_biltrans(bt);	
			print(cur_line, ']\t' + outline, file=tst_srcout);
		elif cur_line in dev: #{
			print(cur_line, 'dev');
			outline = disambiguate_with_alig(bt, al, tl);	
			print(cur_line, ']\t' + outline, file=dev_refout);
			outline = convert_to_biltrans(bt);	
			print(cur_line, ']\t' + outline, file=dev_srcout);
		elif cur_line in train: #{
			#print(cur_line, 'train');
			print(cur_line, '\t' + sl, file=trainout);
			print(cur_line, '\t' + bt, file=trainout);
			print(cur_line, '\t' + tl, file=trainout);
			print(cur_line, '\t' + al, file=trainout);
			print('-------------------------------------------------------------------------------', file=trainout);
		#}
		cur_line = -1;
		state = 0;
		sl = '';
		tl = '';
		al = '';
		bt = '';
		continue;
	#}	
	if cur_line == -1: #{
		cur_line = int(line.split('\t')[0]);
	#}	

	if cur_line > -1 and state == 0: #{
		sl = line.split('\t')[1];
		state = state + 1;
	elif cur_line > -1 and state == 1: #{
		bt = line.split('\t')[1];
		state = state + 1;
	elif cur_line > -1 and state == 2: #{
		tl = line.split('\t')[1];
		state = state + 1;
	elif cur_line > -1 and state == 3: #{
		al = line.split('\t')[1];
		state = state + 1;
	else: #{
		print('Something went wrong!', file=sys.stderr);
	#}
#}
