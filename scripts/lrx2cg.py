#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy;
import xml.etree.ElementTree as etree  ;

tree = etree.parse(sys.argv[1]);

root = tree.getroot()

def cg(parell): #{
	return '("' + str(parell[0]) + '" ' + str(parell[1]) + ')' ;
#}

current_rule = {};
context_counter = 0;
print('DELIMITERS = "<.>" "<!>" "<?>" "<...>" "<¶>" "<:>";');
print('');
print('SECTION');
rule_id = 0;
for element in root: #{
	#print('rule:', file=sys.stderr);
	rule_id = rule_id + 1;
	context_counter = 0;
	current_ops = {};
	centre = 0;
	num_ops = 0;
	for child in element: #{
		if 'lemma' not in child.attrib: child.attrib['lemma'] = '';
		if 'tags' not in child.attrib: child.attrib['tags'] = '';
		#print('  ', context_counter, child.tag, child.attrib['lemma'], child.attrib['tags'], file=sys.stderr);
		current_rule[context_counter] = [(child.attrib['lemma'], child.attrib['tags'])];
		if child.tag == 'or': #{
			current_rule[context_counter] = []; 
			for sub in child: #{
				if 'lemma' not in sub.attrib: sub.attrib['lemma'] = '';
				if 'tags' not in sub.attrib: sub.attrib['tags'] = '';
				#print('     ', sub.tag, sub.attrib['lemma'], sub.attrib['tags'], file=sys.stderr);
				current_rule[context_counter].append((sub.attrib['lemma'], sub.attrib['tags']));
			#}
		else: 
			for sub in child: #{
				if 'lemma' not in sub.attrib: sub.attrib['lemma'] = '';
				if 'tags' not in sub.attrib: sub.attrib['tags'] = '';
				#print('     ', sub.tag, sub.attrib['lemma'], sub.attrib['tags'], file=sys.stderr);
				centre = context_counter;
				current_ops[context_counter] = [(sub.attrib['lemma'], sub.attrib['tags'])];
				num_ops = num_ops + 1;
			#}
		#}
		context_counter = context_counter + 1;
	#}
	if num_ops > 1: #{
		print(rule_id, 'skipping...', file=sys.stderr);
		continue;
	#}
	rule = '';
	first_part = 'SELECT ("' + current_ops[centre][0][0] + '"i ';
	first_part = first_part + current_ops[centre][0][1].replace('.', ' ').replace('*', '') + ') IF ';
	first_part = first_part + '(0 ("<' + current_rule[centre][0][0] + '>"i ';
	first_part = first_part + current_rule[centre][0][1].replace('.', ' ').replace('*', '') + '))';
	first_part = first_part.replace(' )', ')');
	context = '';
	for w in current_rule: #{
		if w > centre: #{
			if len(current_rule[w]) > 1: #{
				context = context + '(' + str(w);
				for y in current_rule[w]: #{
					print(w, w, y, file=sys.stderr);
					context = context + ' ("<' + str(y[0]) + '>"i ';
					context = context + str(y[1].replace('.', ' ').replace('*', ''));
					context = context + ') ';
					context = context + ' OR ';
				#}
				context = context + ')';
			else: #{

				context = context + '(' + str(w) + ' ("<' + str(current_rule[w][0][0]) + '>"i ';
				context = context + str(current_rule[w][0][1].replace('.', ' ').replace('*', ''));
				context = context + ')) ';
			#}
		elif w < centre: #{
			if len(current_rule[w]) > 1: #{
				context = context + '(' + str(w-centre);
				for y in current_rule[w]: #{
					print(w, w-centre, y, file=sys.stderr);
					context = context + ' ("<' + str(y[0]) + '>"i ';
					context = context + str(y[1].replace('.', ' ').replace('*', ''));
					context = context + ') ';
					context = context + ' OR ';
				#}
				context = context + ')';
			else: #{
				context = context + '(' + str(w-centre) + ' ("<' + str(current_rule[w][0][0]) + '>"i ';
				context = context + str(current_rule[w][0][1].replace('.', ' ').replace('*', ''));
				context = context + ')) ';
			#}
		#}
		context = context.replace('OR )', ')').replace('"<>"i ', '');
	#}
	print(first_part, context, ';');
	current_rule = {};
#}
