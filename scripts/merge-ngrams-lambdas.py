#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

import sys
import random


def merge_ngrams_lambdas(ngf, ldf):
    # ngram file
    # lambda file
    # lexicon file

    ngrams = {}
    # lambdas = {}

    for line in open(ngf).readlines():
        # 59763 	poor<adj> in<pr> capital<adj>
        if len(line) < 2:
            continue

        row = line.strip().split('\t')
        if(len(row) < 2):
            row.append('')
        ngid = int(row[0].strip())
        ngrams[ngid] = row[1]

    with open(ldf) as d:
        for line in d:
            # 59176:0 1.00131
            if line.count('@@') > 0:
                continue

            row = line.strip().split('\t')

            l = float(row[2])
            ngid = int(row[1].split(':')[0])
            ngram = ngrams[ngid]

            trad = row[1].split(':')[1]
            token = row[0]
            print(token, '\t', l, '\t', trad, '\t', ngram)


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: merge-ngrams-lambdas.py <ngrams> <lambdas>', file=sys.stderr)
        exit(1)

    merge_ngrams_lambdas(sys.argv[1], sys.argv[2])
