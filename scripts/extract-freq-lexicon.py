#!/usr/bin/python
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import common
import traceback

# Read the corpus, make a note of all ambiguous words, their frequency and their possible translations

# sl_tl[sl_word][tl_word] = tl_freq

# Then we want to make a list of n-grams around the source words, with which target word they want, and the freq.

# ngrams[ngram][tl_word] = freq

# 5 	Please<vblex><inf> rise<n> ,<cm> then<adv> ,<cm> for<pr> this<det><dem> minute<n> 's<gen> silence<n> .<sent>
# 5 	Please<vblex><inf>/Complacer<vblex><inf> rise<n><sg>/aumento<n><m><sg> ,<cm>/,<cm> then<adv>/entonces<adv> ,<cm>/,<cm> for<pr>/para<pr>/durante<pr> this<det><dem><sg>/este<det><dem><GD><sg> minute<n><sg>/minuto<n><m><sg> '<apos>/'<apos> *s/*s silence<n><sg>/silencio<n><m><sg> .<sent>/.<sent>
# 5 	Invitar<vblex> a<pr> todo<prn><tn> a<pr> que<cnjsub> prpers<prn><pro> poner<vblex> de<pr> pie<n> para<pr> guardar<vblex><inf> uno<det><ind> minuto<n> de<pr> silencio<n> .<sent>
# 5 	0-0 4-2 5-3 8-1 9-5 10-6 12-7 13-8 14-9 15-10
# -------------------------------------------------------------------------------

def extract_freq_lexicon(canditates):

    cur_line = 0
    lineno = 0
    sl_tl = {}

    cur_sl_row = []
    cur_tl_row = []
    cur_bt_row = []
    cur_al_row = []

    with open(canditates) as infile:
        for line in infile:
            line = line.strip()
            lineno += 1
            if lineno % 5000 == 0:
                sys.stderr.write('.')
                if lineno % 100000 == 0:
                    sys.stderr.write(str(lineno)+'\n')

                sys.stderr.flush()

            try:
                if line[0] == '-':
                    # Read the corpus, make a note of all ambiguous words, their frequency and their possible translations
                    #
                    # sl_tl[sl_word][tl_word] = tl_freq
                    i = 0
                    for slword in cur_sl_row:
                        if len(cur_bt_row[i]['tls']) > 1:
                            for al in cur_al_row:
                                if al == '':
                                    continue
                                al_sl = int(al.split('-')[1])
                                al_tl = int(al.split('-')[0])
                                if al_sl != i:
                                    continue

                                if al_tl < len(cur_tl_row):
                                    tlword = cur_tl_row[al_tl]
                                else:
                                    tlword = cur_tl_row[-1]
                                    traceback.print_stack()
                                    print("alignment out",
                                          "of",
                                          "range", al_tl,
                                          "not in",
                                          "len(",
                                          cur_tl_row,
                                          ")",
                                          file=sys.stderr)
                                    exit(1)
                                slword = slword
                                if slword not in sl_tl:
                                    sl_tl[slword] = {}

                                if tlword not in sl_tl[slword]:
                                    sl_tl[slword][tlword] = 0

                                sl_tl[slword][tlword] = sl_tl[slword][tlword] + 1
                                # print '+' , slword , tlword , sl_tl[slword][tlword], lineno

                        i = i + 1

                    cur_line = 0
                    continue

                line = line.split('\t')[1]

                if cur_line == 0:
                    cur_sl_row = common.tokenise_tagger_line(line)
                elif cur_line == 1:
                    cur_bt_row = common.tokenise_biltrans_line(line)
                elif cur_line == 2:
                    cur_tl_row = common.tokenise_tagger_line(line)
                elif cur_line == 3:
                    cur_al_row = line.split(' ')

                cur_line = cur_line + 1
            except Exception:
                # print("Error in line", lineno, ":", e, file=sys.stderr)
                traceback.print_exc()
                exit(1)

    for sl in sl_tl:

        newtl = sorted(sl_tl[sl], key=lambda x: sl_tl[sl][x])
        newtl.reverse()
        first = True
        for tl in newtl:
            if tl[0] == '*':
                print('Error: tl word unknown', tl,  file=sys.stderr)
                continue

            first_tag_sl = sl.split('<')[1].split('>')[0].strip()
            first_tag_tl = tl.split('<')[1].split('>')[0].strip()
            if first_tag_sl != first_tag_tl:
                print('Error:', first_tag_sl, '!=',
                      first_tag_tl, file=sys.stderr)
                continue

            if first:
                print(sl_tl[sl][tl], common.wrap(sl), common.wrap(tl), '@')
                first = False
            else:
                print(sl_tl[sl][tl], common.wrap(sl), common.wrap(tl))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: extract-freq-lexicon.py <candidate sent>', file=sys.stderr)
        exit(1)

    extract_freq_lexicon(sys.argv[1])
