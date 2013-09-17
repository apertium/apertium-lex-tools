#include <cwchar>
#include <cstdio>
#include <libgen.h>
#include <cerrno>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <list>
#include <algorithm>
#include <set>
#include <math.h>

/* YASMET 1.0 toolkit Copyright (C) 2002 Franz Josef Och */
#include <fstream> /* This program is free software; you can redistribute */
#include <cmath>   /* it and/or modify it under the terms of the GNU General */
#include <string>  /* Public License. */
#include <hash_map>/* This program is distributed WITHOUT ANY WARRANTY */
#include <iostream>
#include <vector>
#include <functional>
#include <numeric>
#include <ext/hash_set>
#include <ext/hash_map>
#include <assert.h>

using namespace std;
using namespace __gnu_cxx;

typedef double D;
typedef pair<int, double> fea;
typedef vector<fea> vfea;
size_t v = 0;

template<class T> ostream &operator<<(ostream &out, const vector<T>&x)
{
	copy(x.begin(), x.end(), ostream_iterator<T>(out, " "));
	return out << endl;
}

struct Z {
	double k, q, l;
	Z(double a = 0, double b = -1, double c = 0): k(a), q(c), l((b < 0) ? 1 : log(b)) {};
};

ostream &operator << (ostream &o, const Z &x)
{
	return o << x.k << "," << x.l << ',' << x.q << ' ';
}

struct hash_str {
	size_t operator()(const string &s) const {
		return hash<const char *>()(s.c_str());
	}
};

double exp2(double d)
{
	return exp(d);
}

struct event {
	vector<double> Ny;
	size_t y;
	vector<vfea > f;
	vector<double> fs;

	vector<double> &computeProb(const vector<Z>&z, vector<double>&pr) const {
		vector<double>::iterator p = pr.begin(), pb = pr.begin(), pe = pr.end();
		for (vector<vfea >::const_iterator fi = f.begin(); fi != f.end(); ++fi, ++p) {
			*p = 0.0;
			for (vfea::const_iterator j = fi->begin(); j != fi->end(); ++j) {
				*p += z[j->first].l * j->second;
			}
		}
		transform(pb, pe, pb, bind2nd(plus<double>(), -*max_element(pb, pe)));
		transform(pb, pe, pb, exp2);
		transform(pb, pe, pb, bind2nd(divides<double>(), accumulate(pb, pe, 0.0)));
		return pr;
	}
};

int main(int argc, char **argv)
{
	string s;
	vector<pair<event, double> > E;
	bool lN = 0;
	double TRN = 0.0, TST = 0.0;
	bool qt = 0, initmu = 0;
	vector<pair<string, double> > s2f;
	vector<Z> z;
	ifstream *muf = 0; // Input stream of model file
	int mfc = -2, kfl = 0;
	size_t st = 0; // State in input file
	size_t C = 0, it = 0, maxIt = 1000, ts = 0, noF = 0, Es, I, N = 100000000;
	double ssi = 0.0;
	double d, l = 1e30, old_l, w = 0.0, dSmoothN = 0.0, minBetter = 0.01, lt = 0.0, wt = 0.0, F = 0.0;

	for (int i = 1; i < argc; ++i) {
		string si(argv[i]);
		if (si == "-v" || si == "-V") {
			v = 1 + (si == "-V");
		}
		else if (si == "-q")qt = 1;
		else if (si == "-red") mfc = atoi(argv[++i]);
		else if (si == "-iter") maxIt = atoi(argv[++i]);
		else if (si == "-dN") dSmoothN = atof(argv[++i]);
		else if (si == "-deltaPP") minBetter = atof(argv[++i]);
		else if (si == "-lNorm") lN = 1;
		else if (si == "-smooth") ssi = 1.0 / pow(atof(argv[++i]), 2);
		else if (si == "-kw" || si == "-kr") kfl = 1 + (si == "-kr");
		else if (si == "-initmu") initmu = 1;
		else if (argv[i][0] == '-') {
			cerr << "\nUsage: " << argv[0] << "[-v|-V|-red n|-iter n|-dN d|-lNorm"
			     "|-deltaPP dpp][mu]\n none: GIS \n -red: count-based feature sel"
			     "ection\n   mu: test pp\n-iter: number of iterations\n  -dN: smoothing "
			     "of event counts\n-lNorm: length normalisation\n-deltaPP: end criterion"
			     "minimal change of perplexity\n-kw: write K file\n-kr: read K file\n";
			return 0;
		}
		else {
			muf = new ifstream(argv[i]);
		}
	}
	{
		hash_map<string, int, hash_str> f2s;
		event e;
		size_t curY = 0;
		double wi = 1.0;
		s2f.push_back(pair<string, double>("@@@CORRECTIVE-FEATURE@@@", 0.0));
		z.push_back(Z());
		f2s["@@@CORRECTIVE-FEATURE@@@"] = 0;

		if (muf) { // Only called when scoring
			while (*muf >> s >> d) {
				cerr << *muf << endl;
				size_t p = (!f2s.count(s)) ? (f2s[s] = s2f.size()) : (f2s[s]);
				Z k(0, kfl ? 1 : d , 0);
				pair<string, double> sd(s, 0.0);
				if (p < s2f.size()) {
					s2f[p] = sd;
				}
				else {
					s2f.push_back(sd);
				}

				if (p < z.size()) {
					z[p] = k;
				}
				else {
					z.push_back(k);
				}
			}
		}

		cerr << s2f.size() << " " << f2s.size() << " " << z.size() << endl;

		assert(s2f.size() == f2s.size());
		assert(s2f.size() == z.size());

		while (cin >> s) // Read in the training input
			switch (st) {
				case 0:
					C = atoi(s.c_str());
					st = 1;
					break;

				case 1:
					if (s == "TEST") {
						N = E.size();
					}
					else {
						e.f.resize(C);
						e.fs.resize(C);
						e.y = atoi(s.c_str());
						curY = 0;
						st = 2;
						wi = 1.0;
						e.Ny.resize(C);
						for (size_t c = 0; c < C; ++c)e.Ny[c] = (c == e.y) ? (1.0 - dSmoothN) : (dSmoothN / (C - 1));
					}
					break;
					// $ means read an absolute count, # means new column separator
				case 2:
					if (s == "#")st = 3;
					else if (s == "@")st = 4;
					else if (s == "$")st = 5;
					else abort();
					break;
				case 3:
				case 4:
					if (s == "#") {
						if ( ++curY == C ) {
							st = 1;
							{
								E.push_back(make_pair(e, wi));
								e = event();
							}
							if (v == 2) {
								cerr << "E:" << E.size() << " " << s2f.size() << "  \r";
								cerr.flush();
							}
						}
					}
					else {
						double fc = 1.0;
						if (st == 4) {
							string t;
							cin >> t;
							fc = atof(t.c_str());
						}
						if (st == 4 && s == "@") {
							e.Ny[curY] = fc;
						}
						else {
							if (!f2s.count(s)) {
								if ((muf && kfl == 0) || E.size() >= N || kfl == 2) {
									if (v > 1)cerr << "new " << s << " (igd)" << endl;
									break;
								}
								else {
									f2s[s] = s2f.size();
									s2f.push_back(make_pair(s, 0.0));
								}
							}
							if (E.size() < N)s2f[f2s[s]].second += (curY == e.y);
							if (kfl != 1 || curY == e.y)e.f[curY].push_back(make_pair(f2s[s], fc));
							e.fs[curY] += fc;
							if (E.size() < N)F = max(F, e.fs[curY]);
						}
					}
					break;
				case 5:
					wi = atof(s.c_str());
					st = 2;
					break;
			}
		Es = E.size();
		ts = max(int(Es - N), 0);
		N = Es - ts;
		I = s2f.size();
		cerr << "I: " << I << " F: " << F << endl;
		assert(f2s.size() == s2f.size());
	}
	vector<double> p(C);
	assert(z.size() == 1 || z.size() == I);
	z.resize(I);
	if (initmu == 0 && muf && kfl != 2) {
		N = 0;
		ts = E.size();
	}
	for (size_t n = 0; n < Es; ++n) {
		event &en = E[n].first;
		((n < N) ? TRN : TST) += E[n].second;
		for (size_t c = 0; c < C; ++c) {
			if (lN)for (size_t k = 0; k < en.f[c].size(); ++k)en.f[c][k].second /= en.fs[c];
			else en.f[c].push_back(make_pair(0, F - en.fs[c]));
		}
	}
	if (lN)F = 1.0;
	if ( mfc != -2) {
		cout << C;
		for (size_t n = 0; n < Es; ++n) {
			const event &en = E[n].first;
			cout << endl << en.y << " $ " << E[n].second << " @ ";
			for (size_t i = 0; i < C; ++i) {
				cout << "@ " << en.Ny[i] << " ";
				for (size_t j = 0; j < en.f[i].size(); ++j)if (s2f[en.f[i][j].first].second > mfc)
						cout << s2f[en.f[i][j].first].first << " " << en.f[i][j].second << " " ;
				cout << "# ";
			}
		}
		cout << endl;
	}
	else {
		for (size_t i = 0; i < N; ++i) {
			const event &ei = E[i].first;
			double wi = E[i].second;
			for (size_t y = 0; y < ei.Ny.size(); ++y)for (size_t j = 0; j < ei.f[y].size(); ++j)
					z[ei.f[y][j].first].k += wi * ei.Ny[y] * ei.f[y][j].second;
		}
		if (kfl == 1) {
			for (size_t i = 0; i < s2f.size(); ++i) {
				cout << s2f[i].first << " " << z[i].k << endl;
			}
			exit(0);
		}
		for (size_t i = 0; i < I; ++i)if (!z[i].k)noF++;
		if (noF && N)cerr << "I': " << I - noF << endl;
		if (v > 1) {
			cerr << "Expected feature counts: " << z;
		}

		do {
			old_l = l;
			l = 0.0;
			w = 0.0;
			lt = 0.0;
			wt = 0.0;
			double wx = 0.0, wtx = 0.0, lx = 0, ltx = 0;

			for (size_t i = 0; i < I; ++i) {
				z[i].q = 0;
			}

			for (size_t i = 0; i < Es; ++i) {
				const event &ei = E[i].first;
				double wi = E[i].second;
				size_t pos0 = 0;
				for (pos0 = 0; pos0 < ei.Ny.size(); pos0++)if (ei.Ny[pos0] == 0)break;
				ei.computeProb(z, p);
				vector<double>::const_iterator me = max_element(p.begin(), p.end());
				if (!N) {
					cout << me - p.begin() << " " << p;
				}
				if (i < N) {
					for (size_t j = 0; j < C; ++j) {
						const vfea &eifj = ei.f[j];
						double pj = p[j] * wi;
						for (vfea::const_iterator k = eifj.begin(); k != eifj.end(); ++k) {
							z[k->first].q += pj * k->second;
						}
					}
				}
				((i < N) ? l : lt) -= wi * log(p[ei.y]);

				for (size_t y = 0; y < C; ++y)((i < N) ? lx : ltx) -= ei.Ny[y] * wi * log(p[y]);
				((i < N) ? w : wt) += wi * ((p.begin() + ei.y) != me);
				((i < N) ? wx : wtx) += wi * (ei.Ny[me - p.begin()] == 0.0 && (p.begin() + ei.y) != me);
			}
			for (size_t i = 0; i < I; ++i) {
				Z &x = z[i];
				if (x.k) {
					double dl = (log(x.k) - log(x.q)) / F, ddl = 1.0;
					if (ssi != 0.0)for (int iter = 0; iter < 20 && ddl > 1e-10; ++iter, dl -= ddl) {
							double h = x.q * exp(dl * F);
							ddl = (h + (x.l + dl) * ssi - x.k) / (h * F + ssi);
						}
					x.l += dl;
				}
			}
			if (v > 1) {
				cerr << it << ". " << " KLQ:" << z << " " << p << "\n";
			}
			if (TRN && !qt) {
				cerr << it << ". " << "pp: " << exp(l / TRN) << " er: " << w / TRN << " erx: "
				     << wx / TRN << " ppx: " << exp(lx / TRN);
				if (!TST)cerr << endl;
			}
			if (TST) {
				cerr << " " << "tst-pp: " << exp(lt / TST) << " tst-er: " << wt / TST
				     << " tst-erx: " << wtx / TST << " tst-ppx: " << exp(ltx / TST) << endl;
			}
		}
		while (fabs(exp(l / TRN) - exp(old_l / TRN)) > minBetter && it++ < maxIt && N);

		if (N) {
			for (size_t i = 0; i < I; ++i) {
				cout << s2f[i].first << " " << exp(z[i].l - z[0].l) << '\n';
			}
		}
	} //?
}
