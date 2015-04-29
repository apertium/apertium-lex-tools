#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys ;

#perl "$MOSESDECODER/clean-corpus-n.perl" data.$SL-$TL/$CORPUS.tagged $SL $TL "data.$SL-$TL/$CORPUS.tagged-clean" 1 40;


prefix = sys.argv[1];
sl = sys.argv[2];
tl = sys.argv[3];
outfix = sys.argv[4]

sl_f = open(prefix + '.' + sl);
tl_f = open(prefix + '.' + tl);

sl_o = open(outfix + '.' + sl, 'w+');
tl_o = open(outfix + '.' + tl, 'w+');

reading = True;

inlines = 0;
outlines = 0;

while reading: #{

	slline = sl_f.readline();
	tlline = tl_f.readline();

	if not slline and not tlline: #{
		break;
	#}

	if slline.strip() == '' or tlline.strip() == '': #{
		inlines = inlines + 1;
		continue;
	else: #{
		sl_o.write(slline);
		tl_o.write(tlline);
		outlines = outlines + 1;
	#}
	inlines = inlines + 1;
#}

print('in: %d, out: %d' % (inlines, outlines));
