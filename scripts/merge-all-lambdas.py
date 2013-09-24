#!/bin/python

from subprocess import Popen, PIPE, STDOUT
import sys;

if len(sys.argv) != 2:
	print ("Usage:", sys.argv[0], "<path to yasmet>")
	sys.exit(-1); 

tokens = {}
for line in sys.stdin:
	line = line.rstrip();
	row = line.split('\t');
	token = row[0].rstrip();
	if token not in tokens:
		tokens[token] = {}
		tokens[token]['n'] = row[1];
		tokens[token]['events'] = []
	tokens[token]['events'].append(row[2]);
	
for token in tokens:
	print >>sys.stderr, token
	with open('events', 'w') as tmp_file:
		tmp_file.write(tokens[token]['n'] + '\n')
		for event in tokens[token]['events']:
			tmp_file.write(event.rstrip(' ').lstrip(' ') + '\n');

	with open('events', 'r') as infile:
		outfile = open('lambdas', 'w');
		p_red = Popen([sys.argv[1], '-red', '3'], stdin=infile, stdout=outfile, stderr=None);
		p_red.wait();
		outfile.close();
		outfile = open('lambdas', 'r');
		p_class = Popen([sys.argv[1]], stdin=outfile, stdout=PIPE, stderr=None);
		stdout_data = p_class.communicate()[0].replace(' ', "\t").rstrip().split('\n')
		print ('\n'.join(map(lambda x: token + '\t' + x, stdout_data)))
		outfile.close();


	

