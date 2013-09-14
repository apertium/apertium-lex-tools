#!/bin/python

from subprocess import Popen, PIPE, STDOUT
import sys;

tokens = {}
for line in sys.stdin.readlines():
	line = line.rstrip();
	row = line.split('\t');
	token = row[0].rstrip();
	if token not in tokens:
		tokens[token] = {}
		tokens[token]['n'] = row[1];
		tokens[token]['events'] = []
	tokens[token]['events'].append(row[2]);
	
for token in tokens:
	out = tokens[token]['n'] + '\n';
	for event in tokens[token]['events']:
		out += event + '\n';
	out = out.rstrip();
	p_red = Popen(['/home/philip/Apertium/apertium-lex-tools/yasmet', '-red', '0'], stdin=PIPE, stdout=PIPE, stderr=PIPE);
	p_class = Popen(['/home/philip/Apertium/apertium-lex-tools/yasmet'], stdin=PIPE, stdout=PIPE, stderr=PIPE);

	red_data = p_red.communicate(input=out)[0]
	stdout_data = p_class.communicate(input=red_data)[0].replace(' ', "\t").rstrip().split('\n')
	

	print '\n'.join(map(lambda x: token + '\t' + x, stdout_data))
	

