#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys, codecs, copy, commands;

sys.stdin  = codecs.getreader('utf-8')(sys.stdin);
sys.stdout = codecs.getwriter('utf-8')(sys.stdout);
sys.stderr = codecs.getwriter('utf-8')(sys.stderr);

#-45.4576	||	.[][2486 0].[] La diferencia principal es que el sistema apuntado en los garajes no incluye estimaciones de tiempo del trabajo.
#-44.5838	||	.[][2486 1].[] La diferencia principal es que el sistema dirigido en los garajes no incluye estimaciones de tiempo del trabajo.
#-50.2246	||	.[][2487 0].[] El coste de utilizar esta herramienta en una tienda es 350 euros un año, el cual el cliente puede financiar.
#-14.1091	||	.[][2488 0].[] Actualmente, *Audatex está obrando para hacer estas imágenes tridimensionales.
#-14.0531	||	.[][2488 1].[] Actualmente, *Audatex está explotando para hacer estas imágenes tridimensionales.
#-14.8202	||	.[][2488 2].[] Actualmente, *Audatex está funcionando para hacer estas imágenes tridimensionales.
#-13.8124	||	.[][2488 3].[] Actualmente, *Audatex está trabajando para hacer estas imágenes tridimensionales.
#

current_line = 1;
current_trad = {};
for line in sys.stdin.readlines(): #{

	line = line.strip();
	row = line.split('||');
	prob = float(row[0].strip());
	trad = row[1].split('.[]')[2];
	sno = int(row[1].split('.[]')[1].split(' ')[0].strip('[]'));

	if sno != current_line: #{
		max_p = -9000.0;
		best = '';
		for p in current_trad: #{
			if p > max_p: #{
				best = current_trad[p];
				max_p = p;
			#}	
		#}
		print best; 
		#print 'BEST:' , best ; 
		#print '--';
		current_line = sno;
		current_trad = {};
	#}

#	print sno , prob , trad;
	current_trad[prob] = trad;
#}
