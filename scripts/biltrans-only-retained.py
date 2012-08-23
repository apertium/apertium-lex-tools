#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;

lines = [] ;
for line in open(sys.argv[2]).readlines(): #{
	lines.append(int(line.strip()));
#}
print(sys.argv, len(lines), file=sys.stderr);

lineno = 1;

inf = open(sys.argv[1]);
buf = inf.readline();
while buf != '': #{
	if lineno in lines: #{
		print(buf.strip());
	elif lineno not in lines: #{
		print('Line ' + str(lineno) + ' discarded.', file=sys.stderr);
	else: #{
		print('Something weird happened.', file=sys.stderr);
	#}
	
	lineno = lineno + 1;
	buf = inf.readline();
#}

#
#c = inf.read(1);
#buf = '';
#while c: #{
#	if c == '\n': #{
#		if lineno in lines: #{
#			print(buf.strip());
#		elif lineno not in lines: #{
#			print('Line ' + str(lineno) + ' discarded.', file=sys.stderr);
#		else: #{
#			print('Something weird happened.', file=sys.stderr);
#		#}
#		lineno = lineno + 1;
#		buf = '';
#	#}
#
#	buf = buf + c;
#	c = inf.read(1);
##}
##
#for line in open(sys.argv[1]).readlines(): #{
#	if lineno in lines: #{
#		print(line.strip());
#	elif lineno not in lines: #{
#		print('Line ' + str(lineno) + ' discarded.', file=sys.stderr);
#	else: #{
#		print('Something weird happened.', file=sys.stderr);
#	#}
#	lineno = lineno + 1;
##}
