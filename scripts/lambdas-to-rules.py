import sys;
import common;

def wrap (x):
	return '^' + x + '$'

sl_tl_defaults = {}; 
sl_tl = {};

indexes = {};
trad_counter = {}; 
rindex = {};

with open(sys.argv[1]) as d:
	for line in d: #{
		if len(line) < 1: #{
			continue;
		#}
		row = common.tokenize_tagger_line(line);
		sl = wrap(row[0].strip());
		tl = wrap(row[1].strip());
		if tl[1] == '*':
			tl = tl[:-3] + '$'

		if sl not in sl_tl: #{
			sl_tl[sl] = [];
		#}
		if sl not in trad_counter: #{
			trad_counter[sl] = 0;
		#}
		if line.count('@') > 0: #{
			sl_tl_defaults[sl] = tl;
		#}
		sl_tl[sl].append(tl);
		indexes[(sl, tl)] = trad_counter[sl];
		rindex[(sl, trad_counter[sl])] = tl;
		trad_counter[sl] = trad_counter[sl] + 1;

	#}

for pair in rindex: #{
	print(pair[0], pair[1], rindex[pair], file=sys.stderr);
#}

#ability<n> 	 0.25652 	 1 	 ability<n> to<pr>
#ability<n> 	 1.54548 	 0 	 ability<n> to<pr> deliver<vblex><inf>
#ability<n> 	 1.48162 	 0 	 our<det><pos> ability<n> to<pr>

with open(sys.argv[2]) as d:
	for line in d: #{

		row = line.split(' \t '); 
		slword = row[0].strip();
		l = float(row[1]);
		tlid = int(row[2]);
		if (slword, tlid) not in rindex: #{
			print ('(', slword, ',', tlid, ') not in index', file=sys.stderr)
			continue;
		#}
		tlword = rindex[(slword, tlid)];
		context = row[3].strip();
	#	#+ 0.571428571429 14 8 8 	troiñ<vblex>		tourner<vblex>	8
	#+nature<n>	service<n> nature<n>	carácter<n>	3


		print('+ ' + row[1] + '\t' + slword + '\t' + context + '\t' + tlword + '\t1');

	#	print('  <rule weight="%.5f">' % (l));
	#	for c in context.split(' '): #{
	#		if c.count(slword) == 1: #{
	#			print(slword, tlword);
	#		else: #{
	#			print(c);	
	#		#}
	#	#}
	#	print('  </rule>');

	#}
