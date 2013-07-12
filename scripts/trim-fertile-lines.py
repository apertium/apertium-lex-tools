#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;

# This script selects biltrans lines which generate more than a minimum 
# and less than a maximum number of disambiguation paths.
# By default the minimum is set to 2 -- e.g. the sentences should be ambiguous
# in some way, and the maximum is set to 10,000. If you are discarding too
# many, you can increase this.

MIN_SENT = 2;
MAX_SENT = 10000;

if len(sys.argv) > 1 and sys.argv[1] != '': #{
	MAX_SENT = int(sys.argv[1]);
#}

sent_count = 1;
lineno = 1;
lu = '';
c = sys.stdin.read(1);

def process_biltrans_unit(lu): #{

	state = 0;
	for c in lu[1:-1]: #{ 
		#^worth<n><sg>/valor<n><m><sg>$ ^\$<mon>/\$<mon>$^20<num>/20<num>$^*m/*m$ 
		#print c , sl , tl;
		if c == '/': #{
			state = state + 1;
		#}
	#}

	return state;
#}

escaped = False;
reading_word = False;
outline = '';
while c: #{
	if c == '\\': #{
		escaped = True;
		lu = lu + c;
		c = sys.stdin.read(1);
	#}
	if c == '^': #{
		reading_word = True;
	#}
	if c == '$' and escaped == False: #{
		lu = lu + c;
		sent_count = sent_count * process_biltrans_unit(lu);
		reading_word = False;
		lu = '';		
	#}
	if c != '\\' and escaped == True: #{
		escaped = False;
	#}
	if c.isspace(): #{
		if c == '\n': #{
			if sent_count >= MIN_SENT and sent_count < MAX_SENT: #{
				print(str(sent_count) + " " + outline.strip());
			else: #{
				#print('Line ' + str(lineno) + ' has ' + str(sent_count) + ' translations, discarding.', file=sys.stderr);
				print(str(lineno), file=sys.stderr);
			#}
			lineno = lineno + 1;
			sent_count = 1;
			outline = '';
		#}
	#}
	if reading_word: #{
		lu = lu + c;
	#}
	outline = outline + c;
	c = sys.stdin.read(1);
#}
