/*
 * GraphState2.h
 *
 *  Created on: Jun 28, 2011
 *      Author: pickrell
 */

#ifndef GRAPHSTATE2_H_
#define GRAPHSTATE2_H_

#include "Settings.hpp"
#include "PopGraph.h"
#include "CountData.h"
#include "PhyloPop_params.h"

class GraphState2{
public:
	GraphState2();
	GraphState2(CountData*, PhyloPop_params*);

	PhyloPop_params* params; //paramters for run
	PopGraph* tree;
	PopGraph* tree_bk;
	PopGraph* tree_bk2;
	PopGraph* tree_bk3;

	gsl_matrix *sigma;
	gsl_matrix *sigma_cor;

	PopGraph* scratch_tree;
	gsl_matrix *scratch_sigma_cor;

	CountData* countdata; //pointer to the data
	vector<string> allpopnames; //names of populations, will be added one at a time after the first 3
	map<string, int> popname2index; //go from the population name to the index in the above vector
	int current_npops; //current total number of populations
	double current_llik;
	double scatter_det, scatter_gamma;
	gsl_matrix *scatter; //current scatter matrix
	double phi, resphi;

	//set the graph structure to a Newick string
	void set_graph(string);
	void set_graph_from_file(string);
	void set_graph_from_string(string);

	//set graph from files with the vertices and edges
	void set_graph(string, string);

	//set the root to a given clade
	void place_root(string);

	//print
	void print_trimmed(string);

	//covariance matrix
	void compute_sigma();
	void print_sigma();
	void print_sigma_cor(string);

	//local hill-climbing
	int local_hillclimb(int);
	int local_hillclimb_root();
	int many_local_hillclimb();
	void iterate_hillclimb();

	//global hill-climbing
	int global_hillclimb(int);
	int many_global_hillclimb();
	void iterate_global_hillclimb();

	//add a new population
	void add_pop();
	void process_scatter();

	//under normal model, get the max lik branch lengths
	// for a given topology by least squares
	void set_branches_ls();
	void set_branches_ls_wmig();
	void set_branch_coefs(gsl_matrix*, gsl_vector*, map<Graph::edge_descriptor, int>*, map<Graph::edge_descriptor, double>*);
	//functions used by the above least squares fitting
	map<Graph::vertex_descriptor, int> get_v2index();

	//maximize the weights on the branches. This will be iterative on each individual weight
	void optimize_weights();
	void optimize_weight(Graph::edge_descriptor);
	void quick_optimize_weight(Graph::edge_descriptor);
	int golden_section_weight(Graph::edge_descriptor, double, double, double, double);

	void optimize_fracs();
	void optimize_frac(Graph::edge_descriptor);
	void quick_optimize_frac(Graph::edge_descriptor);
	int golden_section_frac(Graph::edge_descriptor, double, double, double, double);

	//likelihoods
	double llik();
	double llik_normal();
	double llik_wishart();

	//migration
	pair<string, string> get_max_resid();
	bool try_mig(Graph::vertex_descriptor, Graph::vertex_descriptor, gsl_matrix*);
	void add_mig();
	pair<bool, pair<int, int> > add_mig_targeted();
	pair< pair<bool, bool>, pair<double, pair<int, int> > > add_mig_targeted(string, string);
	double get_mig_targeted(string, string, set<pair<int, int> >*);
	Graph::vertex_descriptor get_neighborhood(Graph::vertex_descriptor); // get the vertex descriptor at the LCA of the neighborhood of a vertex
	int local_hillclimb_wmig(int);
	int iterate_local_hillclimb_wmig(int);
	void iterate_mig_hillclimb_and_optimweight(pair<int, int>);
	void flip_mig();
	void trim_mig();
	//get newick string with trimmed terminal branch lengths
	string get_trimmed_newick();
	void iterate_movemig(int);
	pair<bool, int> movemig(int);

	//alterations to the tree
	Graph::edge_descriptor add_mig(int, int);
	void rearrange(int, int);

	bool has_loop();
};

#endif /* GRAPHSTATE2_H_ */