/*
 * WishartState.h
 *
 *  Created on: Apr 19, 2011
 *      Author: pickrell
 */

#ifndef WISHARTSTATE_H_
#define WISHARTSTATE_H_

#include "BinaryTree.h"
#include "Settings.hpp"
#include "CountData.h"
#include "MCMC_params.h"

class WishartState{
public:

	/*
	 *  Initializes tree to newick string, count data)
	 */
	WishartState(string, CountData*, MCMC_params*);

	/*
	 * Copy constructor
	 */

	WishartState(const WishartState&);


	// pointer to the tree data structure
	PhyloPop_Tree::Tree<PhyloPop_Tree::NodeData>* tree;

	gsl_matrix *sigma;
	double current_lik; //store the current likelihood
	CountData* countdata; //pointer to the data
	MCMC_params* params; //pointer to the parameters for updates

	//initialize the likelihood
	void init();

	//update the tree
	void update_tree(gsl_rng*);
	vector<PhyloPop_Tree::iterator<PhyloPop_Tree::NodeData> > propose_tree(gsl_rng*);

	// compute the covariance matrix from the tree
	void compute_sigma();
	void print_sigma();
	void read_sigma(string);


	//compute the log-likelihood of the data given the tree
	double llik();
	double dens_wishart();
	void print_state(ogzstream&);
};


#endif /* WISHARTSTATE_H_ */
