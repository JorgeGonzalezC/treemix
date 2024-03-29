/*
 * PhyloPop_wishart.cpp
 *
 *  Created on: Apr 20, 2011
 *      Author: pickrell
 */


#include "Settings.hpp"
#include "MCMC.h"

string infile;
string outstem = "PhyloPop";
int seed = 200;
int nthread = 1;

void printopts(){
	cout << "\nPhyloPop v.0.0 \n by JKP\n\n";
	cout << "Options:\n";
	cout << "-i input file\n";
	cout << "-o output stem (will be [stem].treeout.gz and [stem].meanout.gz)\n";
}


int main(int argc, char *argv[]){
    CCmdLine cmdline;
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

    string treefile = outstem+".treeout.gz";
    ogzstream treeout(treefile.c_str());
    CountData counts(infile);
    string pops = counts.get_pops();
    cout << pops << "\n";


    // MCMC parameters
    MCMC_params p;
    p.nthread = nthread;
    WishartState initstate(pops,  &counts, &p);


    //start random number generator
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    T = gsl_rng_ranlxs2;
    r = gsl_rng_alloc(T);
    if (seed > 0)  gsl_rng_set(r,(int)seed);
    else{
    	seed = (int)time(0);
    	gsl_rng_set(r, seed);
    }

    // initialization
    initstate.tree->randomize_tree(r);
    initstate.compute_sigma();
    initstate.init();
    //MCMC
    MCMC mcmc(&initstate, &p);
    mcmc.run(r, treeout);
	return 0;
}
