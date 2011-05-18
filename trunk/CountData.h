/*
 * CountData.h
 *
 *  Created on: Apr 1, 2011
 *      Author: pickrell
 */

#ifndef COUNTDATA_H_
#define COUNTDATA_H_
#include "Settings.hpp"



class CountData{
public:
	CountData(string, int);
	CountData(string); //no rescaling (ie. just the allele frequencies)
	void read_counts(string);
	map<string, int> pop2id;
	vector<vector<pair<int, int> > > allele_counts;
	int npop, nsnp;
	string get_pops();
	gsl_matrix *alfreqs, *scatter, *cov;
	void set_alfreqs();
	void scale_alfreqs(int);
	void set_scatter();
	void set_cov();
	void process_scatter();
	void process_cov();
	void read_scatter(string); //for debugging
	void print_scatter(string);
	void print_fst(string);
	double scatter_det, scatter_gamma, cov_var;
};

#endif /* COUNTDATA_H_ */