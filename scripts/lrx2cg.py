#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import xml.etree.ElementTree as etree

tree = etree.parse(sys.argv[1])

root = tree.getroot()

def attr(el):
	return (el.attrib.get('lemma', ''), el.attrib.get('tags', '').replace('.', ' ').replace('*', ''))

def ctx(pair, surf=True):
	if isinstance(pair, list):
		return ' OR '.join(ctx(p) for p in pair)
	ls = []
	if pair[0]:
		if surf:
			ls.append('"<%s>"i' % pair[0])
		else:
			ls.append('"%s"i' % pair[0])
	ls += pair[1].split()
	return '(%s)' % (' '.join(ls) or '*')

current_rule = {}
context_counter = 0
print('DELIMITERS = "<.>" "<!>" "<?>" "<...>" "<¶>" "<:>";')
print('')
print('SECTION')
rule_id = 0
for element in root:
	#print('rule:', file=sys.stderr)
	rule_id += 1
	context_counter = 0
	current_ops = {}
	centre = 0
	num_ops = 0
	for child in element:
		current_rule[context_counter] = [attr(child)]
		if child.tag == 'or':
			current_rule[context_counter] = [attr(sub) for sub in child]
		else:
			for sub in child:
				centre = context_counter
				current_ops[context_counter] = [attr(sub)]
				num_ops += 1
		context_counter += 1
	if num_ops > 1:
		print(rule_id, 'skipping...', file=sys.stderr)
		continue
	first_part = 'SELECT %s IF (0 %s)' % (ctx(current_ops[centre][0], False), ctx(current_rule[centre][0]))
	context = ''
	for w, pat in current_rule.items():
		context += ' (%d %s)' % (w-centre if w < centre else w, ctx(pat))
	print(first_part, context, ';')
	current_rule = {}
