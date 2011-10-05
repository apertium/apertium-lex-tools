#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, re;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

global debug;
global trace;
rule_table = {};

class Rule: #{
	tipus = '';
	centre = '';
	tl_patro = [];
	sl_patro = {};
	lineno = 0;
	patro_size = 0;

	def __init__(self, _tipus, _line, _centre, _tl, _sl): #{
		self.tipus = _tipus;
		self.centre = _centre;
		self.tl_patro = _tl;
		self.lineno = _line;
		self.sl_patro = _sl;
		self.patro_size = len(_sl);
	#}

	def show(self): #{

		s = 'rule:',  self.centre , (self.tipus, self.tl_patro) , self.sl_patro;
		return s;
	#}
#}


def loadRulesFromFile(f): #{
	curline = 0;
	for line in file(f).read().split('\n'): #{
		curline = curline + 1;
		if line.count('\t') < 1: #{
			continue;
		#}
		row = line.split('\t');
		tipus = row[0]; # 's' or 'r'
		sl_lema = row[1].split('"')[1];
		sl_tags = row[1].split('"')[2].split(')')[0].strip().replace(' ', '><');
		sl_centre = sl_lema + '<' + sl_tags + '>';
		tl_lema = row[2].split('"')[1];
		tl_tags = row[2].split('"')[2].split(')')[0].strip().replace(' ', '><');
		tl_patro = tl_lema + '<' + tl_tags + '>';
		pattern = row[3];

		sl_patro = {};
		in_context = False;
		in_pattern = False;
		in_lemma = False;
		in_tags = False;
		pos = '';
		lemma = '';
		tags = '';
		for c in pattern.decode('utf-8'): #{
			if c == '(': #{
				in_context = True;	
				in_lemma = False;
				in_pattern = False;
				continue;
			#}
			if c == ')': #{
				lemma = lemma.replace('*', '[\w ]+');
				repatro = lemma + '<' + tags.strip().replace(' ', '><') 
				if repatro.count('><') > 0: repatro = repatro + '>' + '.*';
				#print sl_centre , '>>>' , tl_patro , '>>> ' , pos , repatro;
				sl_patro[int(pos)] = re.compile(repatro);
				pos = '';
				lemma = '';
				tags = '';
				continue;
			#}
			if in_context == True and in_pattern ==  False: #{
				if c == ' ': #{
					in_pattern = True;
					continue;
				else: #{
					pos = pos + c;	
					continue;
				#}
			#}
			if in_context == True and in_pattern == True and in_lemma == False: #{
				if c == '"': #{
					in_lemma = True;
				else: #{
					tags = tags + c;
				#}
			elif in_context == True and in_pattern == True and in_lemma == True: #{
				if c == '"': #{
					in_lemma = False;
				else: #{
					lemma = lemma + c;
				#}
			#}
		#}

		# Rule:
		#   tl_action = ('s', ["lema"])			('s' ["event<n>"])
		#   sl_pattern = [(pos, "lema")] 		[(-3, "guanyador")]

		rule = Rule(tipus, curline, sl_centre, [tl_patro], sl_patro);
		rule.show();
		if sl_centre not in rule_table: #{
			rule_table[sl_centre] = [];
		#}
		rule_table[sl_centre].append(rule);
	#}

	return rule_table;
#}

def procLexicalUnit(c): #{
	global debug;
	escaped = False; 	
	sl = '';
	tl = [];

	c = sys.stdin.read(1);
	while c != '/' and not escaped: #{
		sl = sl + c;
		c = sys.stdin.read(1);
	#}

	buf = '';
	c = sys.stdin.read(1);
	while c != '$': #{
		if c == '\\': 
			escaped = True; 
			buf = buf + c;
			c = sys.stdin.read(1);
			continue;

		if c == '/' and not escaped and buf: 
			tl.append(buf);
			buf = '';
			c = sys.stdin.read(1);
			continue;

		buf = buf + c;
		c = sys.stdin.read(1);
		escaped = False;
	#}
	tl.append(buf);

	return (sl, tl);
#}

# (u'prova<n><f><sg>', [u'proof<n><sg>', u'event<n><sg>', u'exam<n><sg>', u'trial<n><sg>', u'test<n><sg>', u'exhibit<n><sg>', u'testing<n><sg>', u'evidence<n><sg>', u'piece# of evidence<n><sg>']) prova<n> 
#('rule:', 'prova<n>', ('s', 'event<n>'), {-3: '"guanyador"', -2: '"de")'})


def procBlock(sentence): #{
	global trace;
	newsentence = [];
	#print >> sys.stderr ,  sentence;
	i = 0;
	# For each LU in the sentence
	for pair in sentence: #{
		newsentence.append(pair);
		# For each of the possible centres in the rule table (e.g. 'prova<n>')
		for centre in rule_table: #{
			# Check to see if we find the centre in the current LU on the SL side
			if pair[0].find(centre) == 0: #{
				
				# sort rules by length;
				rules = sorted(rule_table[centre], key=lambda rule: rule.patro_size);
				rules.reverse();

				# If we find it then run through each of the rules with that centre
				for rule in rules: #{
					#print pair , centre , rule.show();
					matched = False;
					# For each of the context items in the rule, we check if it is matched
					# in the current sentence. If all matched, then it is True, otherwise False
					for cont in rule.sl_patro.keys(): #{
						contloc = i + cont;
						if contloc >= len(sentence): #{
							continue;
						#}
						#if sentence[contloc][0].find(rule.sl_patro[cont]) == 0: #{
						if rule.sl_patro[cont].match(sentence[contloc][0]): #{
							matched = True;
						else: #{
							matched = False;
						#}
					#}
					#print >> sys.stderr ,  matched , i , rule.sl_patro;
					
					# Now we apply the action!
					if matched: #{
						newx = [];
						if rule.tipus == 's': #{
							if trace: #{
								sys.stderr.write('SELECT:' + str(rule.lineno) + ' ' + str(rule.tl_patro) + '\n');
							#}
							for tl in sentence[i][1]: #{
								for tlf in rule.tl_patro: #{	
									if tl.find(tlf) == 0: #{
										if tl not in newx: #{
											newx.append(tl);
										#}
									#}
								#}
							#}
				
						elif rule.tipus == 'r': #{
							if trace: #{
								sys.stderr.write('REMOVE:' + str(rule.lineno) + ' ' + str(rule.tl_patro) + '\n');
							#}
							for tl in sentence[i][1]: #{
								for tlf in rule.tl_patro: #{	
									if tl.find(tlf) != 0: #{
										if tl not in newx: #{
											newx.append(tl);
										#}
									#}
								#}
							#}
						#}
						del newsentence[i];
						newsentence.append((sentence[i][0], newx));
						break;
					else: #{
						continue;
					#}
				#}
			#}
		#}
		i = i + 1;
	#}
	#print newsentence;
	return newsentence;
#}

def usage(): #{
	print 'apertium-lex-rules.py [-d|-t] <rule file>';
#}

debug = False;
trace = False;

if len(sys.argv) < 2: #{
	usage();
	sys.exit(-1);
#}

if sys.argv[1] == '-d': #{
	debug = True;
	rule_table = loadRulesFromFile(sys.argv[2]);
elif sys.argv[1] == '-t': #{
	trace = True;
	rule_table = loadRulesFromFile(sys.argv[2]);
else: #{
	rule_table = loadRulesFromFile(sys.argv[1]);
#}

#sys.exit(-1);

escaped = False;

c = sys.stdin.read(1);
cur_word = 0;

sent = [];
while c: #{
	if c == '\n': #{
		sent = procBlock(sent);
		for lu in sent: #{
			sys.stdout.write('^' + lu[0] + '/');
			lus = list(set(lu[1]));
			if len(lus) == 0: #{
				sys.stderr.write('Error: output LU count 0\n');
			#}
			for tl in lus: #{
				sys.stdout.write(tl);
				if tl != lus[-1]: #{
					sys.stdout.write('/');
				#}
			#}
			sys.stdout.write('$ ');
		#}
		sent = [];
		sys.stdout.write(c);
	#}
	if c == '\\': 
		escaped = True;
		sys.stdout.write(c);
		c = sys.stdin.read(1);
		continue;

	if c == '^' and not escaped: #{
		sent.append(procLexicalUnit(c));
#	elif c == '[' and not escaped: #{
#		sys.stdout.write(c);
#		while c and c != ']':  #{
#			c = sys.stdin.read(1);
#			sys.stdout.write(c);
#		#}
#	else: #{
#		sys.stdout.write(c);
	#}
		
	c = sys.stdin.read(1);
	escaped = False;
#}
