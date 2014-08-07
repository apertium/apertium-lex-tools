import sys ; 

# IN: ^carefully<adv>/cuidadosamente<adv>$
#     ^whither<adv><*>/dónde<adv><*>$
#     ^Kazakhstan<np><loc><*>/Kazajistán<np><loc><m><*>$
# OUT: 
#      <rule><match lemma="carefully" tags="adv"><select lemma="cuidadosamente" tags="adv"/></match></rule>
#      <rule><match lemma="whither" tags="adv.*"><select lemma="dónde" tags="adv.*"/></match></rule>
#      <rule><match lemma="Kazakhstan" tags="np.loc.*"><select lemma="Kazajistán" tags="np.loc.*"/></match></rule>

defaults = set();

for line in sys.stdin.readlines(): #{
	if line.count('/') < 1: #{
		continue;
	#}
	lemma_tl = '';
	tags_tl = '';
	lemma_sl = '';
	tags_sl = '';
	state = 0;
	escaped = False;
	for c in line: #{
		if c == '^': #{
			state = 1; 
			continue;
		#}
		if c == '\\': #{
			escaped = True;
			continue;
		#}
		if c == '<': #{
			if state == 1: #{
				state = 2; 
			#}
			if state == 3: #{
				state = 4;
			#}
			continue;
		#}
		if c == '/' and state == 2 and not escaped: #{
			state = 3	
			continue;
		#}
		if c == '$' or (c == '/' and state > 2) and not escaped: #{
			break;
		#}

		if state == 1: #{
			lemma_sl = lemma_sl + c;
		elif state == 2: #{	
			if c == '>': #{
				tags_sl = tags_sl + '.'	
			elif c != '<': #{
				tags_sl = tags_sl + c;
			#}
				
		elif state == 3: #{
			lemma_tl = lemma_tl + c;	
		elif state == 4: #{
			if c == '>': #{
				tags_tl = tags_tl + '.'	
			elif c != '<': #{
				tags_tl = tags_tl + c;
			#}
		#}
	#}	
	tags_sl = tags_sl.strip('.');
	tags_tl = tags_tl.strip('.');

	defaults.add((lemma_sl, tags_sl, lemma_tl, tags_tl));
#}

print('<rules>');

rules = list(defaults); 

for rule in rules: #{
	print('  <rule><match lemma="%s" tags="%s"><select lemma="%s" tags="%s"/></match></rule>' % (rule[0], rule[1], rule[2], rule[3]));
#}
print('</rules>');
