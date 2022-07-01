#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import common
import biltrans_count_common as BCC

# Input:

# 0.6000015452	k<post> bukatu<vblex> ari izan<vbper>	bukatu<vblex>	acabar<vblex>
# 0.3999984548	k<post> bukatu<vblex> ari izan<vbper>	bukatu<vblex>	terminar<vblex>
# 0.2435956440	a<det> eta<cnjcoo> bukatu<vblex>	bukatu<vblex>	acabar<vblex>
# 0.7564043560	a<det> eta<cnjcoo> bukatu<vblex>	bukatu<vblex>	terminar<vblex>
# 0.0003531084	eta<cnjcoo> *ed bukatu<vblex> izan<vbsint> n<rel>	bukatu<vblex>	acabar<vblex>
# 0.9996468916	eta<cnjcoo> *ed bukatu<vblex> izan<vbsint> n<rel>	bukatu<vblex>	terminar<vblex>
# 0.4520909033	*Jazten bukatu<vblex>	bukatu<vblex>	acabar<vblex>
# 0.5479090967	*Jazten bukatu<vblex>	bukatu<vblex>	terminar<vblex>

#	 d) Crispiness threshold

def ngram_pruning_frac(lex_freq, ngrams_file, crisphold=3.0):
    cur_line = 0
    only_max = True

    ngrams = {}

    # First read in the frequency defaults
    _, sl_tl_defaults, _ = BCC.read_frequencies(lex_freq)

    max_crispiness = 0.0
    print('Reading...', file=sys.stderr)

    # Load counts from cached file

    ngramsf = open(ngrams_file)
    for line in ngramsf.readlines():
        if len(line) < 1:
            continue

        row = line.split('\t')

        freq = float(row[0])
        ngram = row[1]
        sl = row[2]
        tl = row[3].strip()

        if sl not in ngrams:
            ngrams[sl] = {}

        if ngram not in ngrams[sl]:
            ngrams[sl][ngram] = {}

        if tl not in ngrams[sl][ngram]:
            ngrams[sl][ngram][tl] = 0.0

        ngrams[sl][ngram][tl] = freq


    for sl in ngrams:
        if sl == '':
            continue

        for ngram in ngrams[sl]:
            if ngram == '':
                continue

            total = 0.0
            max_freq = -1.0
            max_tl = ''
            for tl in ngrams[sl][ngram]:

                if ngrams[sl][ngram][tl] > max_freq:
                    max_freq = ngrams[sl][ngram][tl]
                    max_tl = tl

                total = total + ngrams[sl][ngram][tl]

            default = sl_tl_defaults[sl]

            if max_tl not in ngrams[sl][ngram] and default not in ngrams[sl][ngram]:
                print('Some shit went down..', file=sys.stderr)
                print('= %s\t%s\t%s' % (sl, ngram, max_tl), file=sys.stderr)
                continue

            if max_freq == 0.0:
                continue

            if only_max == True:
                crispiness = 0.0
                alt_crisp = float(ngrams[sl][ngram][max_tl]) / float(total)
                def_crisp = 1.0
                if default in ngrams[sl][ngram]:
                    def_crisp = float(ngrams[sl][ngram][default] / float(total))

                if def_crisp == 0.0:
                    print('!!! Something wanky happened. :(', file=sys.stderr)
                    print('%.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (
                        total, max_freq, ngrams[sl][ngram][max_tl], sl, ngram, max_tl, ngrams[sl][ngram][max_tl]), file=sys.stderr)
                    print('\tskipping...', file=sys.stderr)
                    continue

                weight = float(ngrams[sl][ngram][max_tl]) / float(total)
                crispiness = alt_crisp/def_crisp

                print('- %.10f %.10f %.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (crispiness, weight, total,
                                                                            max_freq, ngrams[sl][ngram][max_tl], sl, ngram, max_tl, ngrams[sl][ngram][max_tl]))
                # print('- %.10f \t%s\t%s\t%s\t%.10f' % (crispiness, sl, ngram, max_tl, ngrams[sl][ngram][max_tl]));

                if crispiness > max_crispiness:
                    max_crispiness = crispiness


            #   crispiness   weight      total default     max_freq     tl_freq            sl
            # + 2.61845457309 0.7236389238 1.0 0.2763610762 0.7236389238 0.7236389238         aozer<n>        aozer<n> an<det> levr<n>        organisateur<n> 0.7236389238
            # - 14736.0468727 0.9999321438 1.0 0.9999321438 0.9999321438      treuzkas<n>     treuzkas<n> teknologel<adj>     transfert<n>    0.9999321438
            else:

                for tl in ngrams[sl][ngram]:

                    crispiness = 0.0
                    default = sl_tl_defaults[sl]
                    alt_crisp = float(ngrams[sl][ngram][tl]) / float(total)
                    def_crisp = 1.0
                    if default in ngrams[sl][ngram]:
                        def_crisp = float(
                            ngrams[sl][ngram][default] / float(total))

                    weight = float(ngrams[sl][ngram][tl]) / float(total)
                    crispiness = alt_crisp/def_crisp

                    # print '%%%' , crispiness , alt_crisp , def_crisp , tl , default , ngrams[sl][ngram] ;

                    print('- %.10f %.10f %.10f %.10f %.10f %.10f\t%s\t%s\t%s\t%.10f' % (crispiness, weight, total,
                                                                                        ngrams[sl][ngram][default], max_freq, ngrams[sl][ngram][tl], sl, ngram, tl, ngrams[sl][ngram][tl]))
                    # + 1013.01568891 0.9989973752 2.0 1.9979947504 1.9979947504 	galloud<n>	ha<cnjcoo> an<det> galloud<n>	puissance<n>	1.9979947504

                if crispiness > max_crispiness:
                    max_crispiness = crispiness


    print('max_crispiness: %.10f' % (max_crispiness), file=sys.stderr)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: ngram-pruning-frac.py <lex_freq> <ngrams> [crisphold]', file=sys.stderr)
        exit(1)
    
    if len(sys.argv) == 4:
        print('crisp:', sys.argv[3], file=sys.stderr)
        ngram_pruning_frac(sys.argv[1], sys.argv[2], sys.argv[3])
    else:
        ngram_pruning_frac(sys.argv[1], sys.argv[2])
