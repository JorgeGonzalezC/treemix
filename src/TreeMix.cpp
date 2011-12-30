/*
 * TreeMix.cpp
 *
 *  Created on: Apr 12, 2011
 *      Author: pickrell
 */

#include "Settings.hpp"
#include "CountData.h"
#include "GraphState2.h"
#include "PhyloPop_params.h"

string infile;
string outstem = "TreeMix";

void printopts(){
	cout << "\nTreeMix v. 1.0 \n";
	cout << "Options:\n";
	cout << "-i [file name] input file\n";
	cout << "-o [stem] output stem (will be [stem].treeout.gz, [stem].cov.gz, [stem].modelcov.gz)\n";
	cout << "-k [int] number of SNPs per block for estimation of covariance matrix (1)\n";
	cout << "-global Do a round of global rearrangements after adding all populations\n";
	cout << "-tf [file name] Read the tree topology from a file, rather than estimating it\n";
	cout << "-m [int] number of migration edges to add (0)\n";
	cout << "-root [string] comma-delimited list of populations to set on one side of the root (for migration)\n";
	cout << "-g [vertices file name] [edges file name] read the graph from a previous TreeMix run\n";
	cout << "-se Calculate standard errors of migration weights (computationally expensive)\n";
	cout << "\n";
}


int main(int argc, char *argv[]){

	const gsl_rng_type * T;
	gsl_rng * r;
	gsl_rng_env_setup();
	T = gsl_rng_ranlxs2;
	r = gsl_rng_alloc(T);
	int seed = (int) time(0);
	gsl_rng_set(r, seed);

    CCmdLine cmdline;
    PhyloPop_params p;
    if (cmdline.SplitLine(argc, argv) < 1){
    	printopts();
    	exit(1);
    }
    if (cmdline.HasSwitch("-i")) infile = cmdline.GetArgument("-i", 0).c_str();
    else{
    	printopts();
    	exit(1);
    }
    if (cmdline.HasSwitch("-o"))	outstem = cmdline.GetArgument("-o", 0).c_str();
    if (cmdline.HasSwitch("-tf"))	{
    	p.treefile = cmdline.GetArgument("-tf", 0).c_str();
    	p.readtree = true;
    }
    if (cmdline.HasSwitch("-g"))	{
      	p.vfile = cmdline.GetArgument("-g", 0);
      	p.efile = cmdline.GetArgument("-g", 1);
      	p.read_graph = true;
      }
    if (cmdline.HasSwitch("-arcsin")) p.alfreq_scaling = 1;
    if (cmdline.HasSwitch("-nofrac")) p.nofrac = true;
    if (cmdline.HasSwitch("-scale")) p.alfreq_scaling = 3;
    if (cmdline.HasSwitch("-nothing")) p.alfreq_scaling = 4;
    if (cmdline.HasSwitch("-quick")) p.quick = true;
    if (cmdline.HasSwitch("-global")) p.global = true;
    if (cmdline.HasSwitch("-penalty")) p.neg_penalty = atof(cmdline.GetArgument("-penalty", 0).c_str());;
    if (cmdline.HasSwitch("-se")) p.calc_se = true;
    if (cmdline.HasSwitch("-k"))	p.window_size = atoi(cmdline.GetArgument("-k", 0).c_str());
    if (cmdline.HasSwitch("-m"))	p.nmig = atoi(cmdline.GetArgument("-m", 0).c_str());
    if (cmdline.HasSwitch("-r"))	p.nrand = atoi(cmdline.GetArgument("-r", 0).c_str());
    if (cmdline.HasSwitch("-hold")){
    	string tmp = cmdline.GetArgument("-hold", 0);
    	vector<string> strs;
    	boost::split(strs, tmp, boost::is_any_of(","));
    	for(vector<string>::iterator it = strs.begin(); it!= strs.end(); it++) 	p.hold.insert(*it);

    }
    if (cmdline.HasSwitch("-nf2"))	{
    	p.f2 = false;
    	p.alfreq_scaling = 0;
    }
    if (cmdline.HasSwitch("-root")) {
    	p.set_root = true;
    	p.root = cmdline.GetArgument("-root", 0);
    }
    string treefile = outstem+".treeout.gz";
    string covfile = outstem+".cov.gz";
    string modelcovfile = outstem+".modelcov.gz";
    string cov_sefile = outstem+".covse.gz";
    string llikfile = outstem+".llik";
    ofstream likout(llikfile.c_str());

    //p.bias_correct = false;
    ogzstream treeout(treefile.c_str());
    CountData counts(infile, &p);

    counts.print_cov(covfile);
    counts.print_cov_var(cov_sefile);
    //counts.print_cov_samp("test.gz");
    if (p.smooth_lik) p.smooth_scale = 1; //sqrt( (double) counts.nsnp / (double) p.window_size);
    GraphState2 state(&counts, &p);
    cout.precision(8);
    if (p.readtree) {
    	state.set_graph_from_file(p.treefile);

    	//state.iterate_hillclimb();
    }
    else if (p.read_graph){
    	state.set_graph(p.vfile, p.efile);
    	cout << "Set tree to: "<< state.tree->get_newick_format() << "\n";
    	while (state.current_llik <= -DBL_MAX){
    		cout << "RESCALING\n"; cout.flush();
    		p.smooth_scale = p.smooth_scale *2;
    		state.current_llik = state.llik();
    	}
    	cout << "ln(lk) = " << state.current_llik << " \n";
    }

    while (!p.readtree && counts.npop > state.current_npops){
    	state.add_pop();
    	state.iterate_hillclimb();
    	cout << "ln(likelihood): "<< state.current_llik << " \n";
    	cout << state.tree->get_newick_format() << "\n";
    }
    if (p.global){
    	cout << "Testing global rearrangements\n"; cout.flush();
    	state.iterate_global_hillclimb();
    	if (p.f2) state.set_branches_ls_f2();
    	else state.set_branches_ls_wmig();
    }
    if (p.set_root) state.place_root(p.root);
    likout << "Tree likelihood: "<< state.llik() << " \n";

    for (int i = 0; i < p.nmig; i++){
    	state.current_llik = state.llik();
       	while (state.current_llik <= -DBL_MAX){
       		cout << "RESCALING\n"; cout.flush();
       		p.smooth_scale = p.smooth_scale *2;
       		state.current_llik = state.llik();
       	}
    	double current_nsum = state.negsum;
    	pair<bool, pair<int, int> > add;
    	if (p.f2) add = state.add_mig_targeted_f2();
    	else state.add_mig_targeted();
    	//cout << "here\n"; cout.flush();
    	if (add.first == true) {
    		cout << "Migration added\n";
    		state.iterate_mig_hillclimb_and_optimweight(add.second, current_nsum);
    	}
    	state.optimize_weights_quick();
    	state.trim_mig();

    	if (p.f2) state.set_branches_ls_f2();
    	else state.set_branches_ls_wmig();
    	p.smooth_scale = 1;
    	cout << "ln(likelihood):" << state.llik() << " \n";
    }
	state.clean_negedge();
    treeout << state.tree->get_newick_format() << "\n";
    if (p.sample_size_correct == false) treeout << state.get_trimmed_newick() << "\n";
    //state.current_llik_w = state.llik_wishart();
    //state.optimize_fracs_wish();
    //state.optimize_weights_wish();
    pair<Graph::edge_iterator, Graph::edge_iterator> eds = edges(state.tree->g);
    Graph::edge_iterator it = eds.first;
    p.smooth_lik = false;
    while (it != eds.second){
    	if ( state.tree->g[*it].is_mig){
     		double w = state.tree->g[*it].weight;

     		treeout << state.tree->g[*it].weight<< " ";
     		if (p.calc_se){
        		Graph::vertex_descriptor p1 = source( *it, state.tree->g);
         		p1 = state.tree->get_child_node_mig(p1);
         		Graph::vertex_descriptor p2 = target(*it, state.tree->g);
     			cout << state.tree->get_newick_format(p1) << " ";
     			cout << state.tree->get_newick_format(p2) << "\n"; cout.flush();
     			p.neg_penalty = 0;
     			pair<double, double> se = state.calculate_se(*it);
     			treeout << se.first << " "<< se.second << " ";
     			double test = se.first/ se.second;
     			double pval = 1-gsl_cdf_gaussian_P(test, 1);
     			treeout << pval << " ";
     		}
     		else treeout << "NA NA NA ";

     		state.tree->g[*it].weight = w;
     		if (p.f2) state.set_branches_ls_f2();
     		else state.set_branches_ls_wmig();

     		Graph::vertex_descriptor p1 = source( *it, state.tree->g);
     		p1 = state.tree->get_child_node_mig(p1);
     		Graph::vertex_descriptor p2 = target(*it, state.tree->g);
     		treeout << state.tree->get_newick_format(p1) << " ";
     		treeout << state.tree->get_newick_format(p2) << "\n";
    	}
		it++;
    }
    if (p.sample_size_correct == true) state.tree->print(outstem);
    else state.print_trimmed(outstem);
    state.print_sigma_cor(modelcovfile);


	return 0;
}
