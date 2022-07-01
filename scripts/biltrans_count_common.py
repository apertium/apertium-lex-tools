#!/usr/bin/python3
# coding=utf-8
# -*- encoding: utf-8 -*-

from collections import defaultdict
import common
import math
import re
import sys


def safe_float(s):
    if not s:
        return 0.0, False
    try:
        f = float(s)
        if math.isnan(f):
            return 0.0, False
        return f, True
    except:
        return 0.0, False


def strip_tags(s, side, space=False):
    idx = 0 if side == 'sl' else 1
    ret = s.split('/')[idx]
    if not space:
        ret = ret.replace(' ', '~')
    if ret.count('><') > 0:
        ret = ret.split('><')[0] + '>'
    return ret


class BiltransCounter:
    lu_sep = re.compile(r'\$[^\^]*\^')
    tokenizer = 'regex'  # or 'biltrans'
    line_ids = False
    count_ngrams = False
    max_ngrams = 3
    biltrans_wrap_lus = False

    def __init__(self):
        self.reading = False

        self.am_file = None
        self.am_line = None
        self.am_row = None
        self.am_id = None
        self.am_linenum = 0
        self.dm_file = None
        self.dm_line = None
        self.dm_row = None
        self.dm_id = None
        self.dm_linenum = 0

        self.clear_ngrams()

    def __del__(self):
        if self.am_file:
            self.am_file.close()
        if self.dm_file:
            self.dm_file.close()

    def next_am_line(self):
        self.am_line = self.am_file.readline()
        self.am_linenum += 1
        if not self.am_line:
            self.am_id, self.am_row = None, []
            self.reading = False
            return
        ls = self.am_line.split('\t')
        if self.line_ids:
            self.am_id = int(ls[0].strip())
        if self.tokenizer == 'regex':
            self.am_row = self.lu_sep.split(ls[1].strip()[1:-1])
        elif self.tokenizer == 'biltrans':
            self.am_row = common.tokenize_biltrans_line(self.am_line)

    def next_dm_line(self):
        self.dm_linenum += 1
        self.dm_line = self.dm_file.readline()
        if not self.dm_line:
            self.dm_id, self.dm_row = None, []
            self.reading = False
            return
        ls = self.dm_line.split('\t')
        if self.line_ids:
            self.dm_id = int(self.dm_line.split('.[][')[1].split()[0])
        if self.tokenizer == 'regex':
            self.dm_row = self.lu_sep.split(ls[1].strip()[1:-1])
        elif self.tokenizer == 'biltrans':
            self.dm_row = common.tokenize_biltrans_line(self.dm_line)

    def clear_ngrams(self):
        self.ngrams = defaultdict(
            lambda: defaultdict(lambda: defaultdict(lambda: 0.0)))

    def check_rows(self):
        if len(self.am_row) != len(self.dm_row):
            print(
                'Mismatch in number of LUs between analysis and training', file=sys.stderr)
            print('\t' + self.am_line, file=sys.stderr)
            print('\t' + self.dm_line, file=sys.stderr)
            print('...skipping', file=sys.stderr)
            return False
        return True

    def read_files_multi_dm(self, am_fname, dm_fname):
        self.next_dm_line()
        while self.reading:
            self.next_am_line()
            while self.am_id == self.dm_id and self.reading:
                frac_count = 0
                if self.dm_line.count('\t') > 1:
                    frac_count, _ = safe_float(self.dm_line.split('\t')[2])
                if self.check_rows():
                    self.process_row(frac_count)
                self.next_dm_line()
            if self.am_linenum % 1000 == 0:
                print('=> %d SL and %d TL lines read' %
                      (self.am_linenum, self.dm_linenum), file=sys.stderr)

    def read_files(self, am_fname, dm_fname):
        self.am_file = open(am_fname)
        self.dm_file = open(dm_fname)
        self.reading = True
        if self.line_ids:
            self.read_files_multi_dm(am_fname, dm_fname)
            return
        while self.reading:
            self.next_am_line()
            self.next_dm_line()
            if self.reading and self.check_rows():
                self.process_row()
            if self.am_linenum % 1000 == 0:
                print('=> %d lines read' % self.am_linenum, file=sys.stderr)

    def process_row(self, frac_count=0):
        if self.tokenizer == 'regex':
            cur_sl_row = [strip_tags(s, 'sl', space=True) for s in self.am_row]
            for i in range(len(self.am_row)):
                if self.am_row[i].count('/') > 1:
                    sl = strip_tags(self.am_row[i], 'sl')
                    tl = strip_tags(self.dm_row[i], 'tl')
                    self.process_lu_internal(sl, tl, i, cur_sl_row, frac_count)
        elif self.tokenizer == 'biltrans':
            cur_sl_row = [x['sl'] for x in self.am_row]
            for i in range(len(self.am_row)):
                if len(self.am_row[i]['tls']) > 1:
                    sl = self.am_row[i]['sl']
                    tl = self.dm_row[i]['tls'][0]
                    if self.biltrans_wrap_lus:
                        sl = common.wrap(sl)
                        tl = common.wrap(tl)
                    self.process_lu_internal(sl, tl, i, cur_sl_row, frac_count)

    def process_lu_internal(self, sl, tl, idx, cur_sl_row, frac_count=0):
        if self.count_ngrams:
            for j in range(1, self.max_ngrams):
                pregram = ' '.join(map(common.wrap, cur_sl_row[idx-j:idx+1]))
                postgram = ' '.join(map(common.wrap, cur_sl_row[idx:idx+j+1]))
                roundgram = ' '.join(
                    map(common.wrap, cur_sl_row[idx-j:idx+j+1]))
                self.ngrams[sl][pregram][tl] += frac_count
                self.ngrams[sl][postgram][tl] += frac_count
                self.ngrams[sl][roundgram][tl] += frac_count
        self.process_lu(sl, tl, idx, cur_sl_row, frac_count)

    def process_lu(self, sl, tl, idx, cur_sl_row, frac_count=0):
        pass


def features_and_outline(ngrams, sl, tl, sl_tl, features, indexes,
                         frac_count=None):
    if not ngrams[sl]:
        return
    meevents = []
    for ni in ngrams[sl]:
        if ni not in features:
            feature_counter = len(features) + 1
            features[ni] = feature_counter
        meevents.append(features[ni])
    if sl not in sl_tl or len(sl_tl[sl]) < 2:
        return
    outline = str(indexes[(sl, tl)])
    if frac_count != None:
        outline += ' $ ' + str(int(frac_count * 10000)) + ' '
    outline += ' # '
    for j in range(len(sl_tl[sl])):
        for feature in meevents:
            outline += '%s:%s ' % (feature, j)
        outline += ' # '
    print('%s\t%s\t%s' % (sl, len(sl_tl[sl]), outline))


def read_frequencies(fname):
    with open(fname) as fin:
        sl_tl = {}
        sl_tl_defaults = {}
        indexes = {}
        trad_counter = defaultdict(lambda: 0)
        for line_ in fin.readlines():
            line = line_.strip()
            if not line:
                continue
            row = common.tokenize_tagger_line(line)
            sl = row[0]
            tl = row[1]
            fr = float(line.split(' ')[0])
            indexes[(sl, tl)] = trad_counter[sl]
            trad_counter[sl] += 1
            if '@' in line:
                sl_tl_defaults[sl] = tl
                if fr == 0.0:
                    print(
                        '!!! Prolly something went wrong here, the default has freq of 0.0', file=sys.stderr)
                else:
                    print('    %s => %s = %.10f' %
                          (sl, tl, fr), file=sys.stderr)
            else:
                sl_tl[sl] = tl
        return sl_tl, sl_tl_defaults, indexes
