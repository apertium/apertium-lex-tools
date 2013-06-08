#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, random;


## Input

# 12 	^gant<pr>/avec<pr>$ ^Bruno<np><ant><m><sg>/Bruno<np><ant>$ ^*Mauguin/*Mauguin$ ^,<cm>/,<cm>$ ^bezañ<vblex><pri><p3><sg><@+FMAINV>/être<vblex><pri><p3><sg><@+FMAINV>$ ^e karg eus<pr>/en charge de<pr>$ ^hon<det><pos><mf><sp>/notre<det><pos><mf><ND>$ ^*planetariom/*planetariom$ ^,<cm>/,<cm>$ ^e<vpart><obj>/@e<vpart><obj>$ ^bezañ<vblex><pii><p3><sg><@+FMAINV>/être<vblex><pii><p3><sg><@+FMAINV>$ ^bezañ<vblex><pp>/être<vblex><pp>$ ^kinnig<vblex><pp>/offrir<vblex><pp>/déposer<vblex><pp>/proposer<vblex><pp>/présenter<vblex><pp>$ ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^toullad<n><m><sg>/tas<n><m><sp>$ ^prezegenn<n><f><pl>/discours<n><m><sp>$ ^kaout<vblex><pri><p3><pl><@+FMAINV>/avoir<vblex><pri><p3><pl><@+FMAINV>$ ^ober# berzh<vblex><pp>/avoir# du succès<vblex><pp>$ ^forzh<pr>/beaucoup de<pr>$ ^pegement<adv><itg>/combien<adv><itg>$ ^.<sent>/.<sent>$

#.[][12 0].[]	^gant<pr>/avec<pr>$ ^Bruno<np><ant><m><sg>/Bruno<np><ant>$ ^*Mauguin/*Mauguin$ ^,<cm>/,<cm>$ ^bezañ<vblex><pri><p3><sg><@+FMAINV>/être<vblex><pri><p3><sg><@+FMAINV>$ ^e karg eus<pr>/en charge de<pr>$ ^hon<det><pos><mf><sp>/notre<det><pos><mf><ND>$ ^*planetariom/*planetariom$ ^,<cm>/,<cm>$ ^e<vpart><obj>/@e<vpart><obj>$ ^bezañ<vblex><pii><p3><sg><@+FMAINV>/être<vblex><pii><p3><sg><@+FMAINV>$ ^bezañ<vblex><pp>/être<vblex><pp>$ ^kinnig<vblex><pp>/proposer<vblex><pp>$ ^un<det><ind><sp>/un<det><ind><GD><ND>$ ^toullad<n><m><sg>/tas<n><m><sp>$ ^prezegenn<n><f><pl>/discours<n><m><sp>$ ^kaout<vblex><pri><p3><pl><@+FMAINV>/avoir<vblex><pri><p3><pl><@+FMAINV>$ ^ober# berzh<vblex><pp>/avoir# du succès<vblex><pp>$ ^forzh<pr>/beaucoup de<pr>$ ^pegement<adv><itg>/combien<adv><itg>$ ^.<sent>/.<sent>$^.<sent>/.<sent>$	0.9002465671	|@|

# num_lines

bt_file = open(sys.argv[1]);
fq_file = open(sys.argv[2]);
num_lines = int(sys.argv[3]); 
bto_file = open(sys.argv[4], 'w+');
fqo_file = open(sys.argv[5], 'w+');

# read in all possible line ids 
# unsort them
# take top num_lines
# for each line in biltrans, output if in num_lines
# for each line in frac, output if in num_lines

line_ids = [];

for line in bt_file.readlines(): #{
	row = line.split('\t');
	lid = int(row[0]);
	line_ids.append(lid);
#}

bt_file.close();

random.shuffle(line_ids);
lines = line_ids[0:num_lines];
lines.sort();

bt_file = open(sys.argv[1]);

outlines = 0;
for line in bt_file.readlines(): #{
	line_id = int(line.split('\t')[0]);
	if line_id in lines: #{
		print(line.strip(), file=bto_file);
		outlines += 1;
	#}
#}

print('Output %d lines to sub biltrans' % (outlines), file=sys.stderr);

outlines = 0;
for line in fq_file.readlines(): #{
	# .[][12 0].[]
	line_id = int(line.split(' ')[0].replace('.[][', ''));	 
	if line_id in lines: #{
		print(line.strip(), file=fqo_file);
		outlines += 1;
	#}
#}

print('Output %d lines to sub frac counts' % (outlines), file=sys.stderr);
