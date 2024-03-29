/*
 * WishartState.cpp
 *
 *  Created on: Apr 19, 2011
 *      Author: pickrell
 */

#include "WishartState.h"


WishartState::WishartState(string newick, CountData* counts, MCMC_params* p){
	countdata = counts;
	params =p;
	tree = new PhyloPop_Tree::BinaryTree<PhyloPop_Tree::NodeData>(newick, countdata->pop2id);
	sigma = gsl_matrix_alloc(counts->npop, counts->npop);
	gsl_matrix_set_zero(sigma);
}



WishartState::WishartState(const WishartState& oldstate){
	tree = oldstate.tree->copy();
	countdata = oldstate.countdata;
	params = oldstate.params;
	sigma = gsl_matrix_alloc(oldstate.countdata->npop, oldstate.countdata->npop);
	gsl_matrix_memcpy(sigma, oldstate.sigma);


}



void WishartState::print_sigma(){
	for(int i = 0; i < countdata->npop; i++){
		for (int j = 0; j < countdata->npop; j++){
			cout << gsl_matrix_get(sigma, i, j) << " ";
		}
		cout << "\n";
	}
}

void WishartState::init(){
	current_lik = llik();
}


void WishartState::compute_sigma(){
	map<int, PhyloPop_Tree::iterator<PhyloPop_Tree::NodeData> > id2node = tree->get_tips(tree->getRoot());
	for( int i = 0; i < countdata->npop; i++){
		for (int j = i; j < countdata->npop; j++){
				if (i == j){
				double dist = tree->get_dist_to_root(id2node[i]);
				gsl_matrix_set(sigma, i, j, dist);
			}
			else{
				PhyloPop_Tree::iterator<PhyloPop_Tree::NodeData> lca = tree->get_LCA(tree->getRoot(), id2node[i], id2node[j]);
				double dist = tree->get_dist_to_root(lca);
				gsl_matrix_set(sigma, i, j, dist);
				gsl_matrix_set(sigma, j, i, dist);
			}
		}
	}
}

void WishartState::read_sigma(string infile){
	gsl_matrix_free(sigma);
	vector<vector<double> > tmpcov;

	ifstream in(infile.c_str());
    vector<string> line;
    struct stat stFileInfo;
    int intStat;
    string st, buf;
    intStat = stat(infile.c_str(), &stFileInfo);
    if (intStat !=0){
            std::cerr<< "ERROR: cannot open file " << infile << "\n";
            exit(1);
    }
    while(getline(in, st)){
             buf.clear();
             stringstream ss(st);
             line.clear();
             while (ss>> buf){
                     line.push_back(buf);
             }
             vector<double> tmp;
             for (vector<string>::iterator it = line.begin(); it != line.end(); it++) tmp.push_back(atof(it->c_str()));
             tmpcov.push_back(tmp);
    }
    int npop = countdata->npop;
    sigma = gsl_matrix_alloc(npop, npop);
    for (int i = 0; i < npop; i++){
    	for (int j = 0; j< npop; j++) gsl_matrix_set(sigma, i, j, tmpcov[i][j]);
    }

}
double WishartState::llik(){
	return dens_wishart();
}


void WishartState::print_state(ogzstream& treefile){
	string t = tree->get_newick_format();
	treefile << t << "\n";
}


void WishartState::update_tree(gsl_rng* r){
	double oldlik = current_lik;

	//flip the nodes
	tree->flip_sons(tree->getRoot(), r);

	//copy the old tree
	PhyloPop_Tree::Tree<PhyloPop_Tree::NodeData>* oldtree = tree->copy();

	//propose new tree
	vector<PhyloPop_Tree::iterator<PhyloPop_Tree::NodeData> > trav = propose_tree(r);

	//check to make sure the maximum distance to the root is not greater than B
	double maxdist = 0;
	for(int i = 0; i < trav.size(); i++){
		if (trav[i]->m_time > maxdist) maxdist = trav[i]->m_time;
	}

	//if it's ok, do a metropolis update
	if (maxdist < params->B){
		double newlik = llik();
		double ratio = exp(newlik-oldlik);
		if (ratio < 1){
			double acc = gsl_rng_uniform(r);
			if (acc > ratio){
				delete tree;
				tree = oldtree;
				compute_sigma();
			}
			else{
				delete oldtree;
				current_lik = newlik;
			}
		}
		else{
			delete oldtree;
			current_lik = newlik;
		}
	}

	//if not, switch back to the old tre
	else{
		delete tree;
		tree = oldtree;
		compute_sigma();
	}
}

vector<PhyloPop_Tree::iterator<PhyloPop_Tree::NodeData> > WishartState::propose_tree(gsl_rng* r){

	//2. get the traversal
	//cout << "get traversal 1\n"; cout.flush();
	vector<PhyloPop_Tree::iterator<PhyloPop_Tree::NodeData> > trav = tree->get_inorder_traversal(countdata->npop);
	//3. fiddle with read depths
	//cout << "perturb\n"; cout.flush();
	tree->perturb_node_heights(trav, params->epsilon, r);
	//4. rebuild the tree
	//cout << "rebild\n"; cout.flush();
	tree->build_tree(trav);
	//5. update the branch lengths
	//cout << "update branch lengths\n"; cout.flush();
	tree->update_branch_lengths(tree->getRoot());
	//6. get the new traversal
	//cout << "traverse 2\n"; cout.flush();
	trav = tree->get_inorder_traversal(countdata->npop);
	//7. reset the heights
	//cout << "reset heights\n"; cout.flush();
	tree->set_node_heights(trav);
	//8. recompute the correlation matrix
	//cout << "compute sigma\n"; cout.flush();
	compute_sigma();
	//cout << "done \n"; cout.flush();
	return trav;
}


double WishartState::dens_wishart(){
	// density of the wishart distribution with covariance matrix sigma, n = number of snps-1, p = number of populations
	// 		scatter matrix has been stored in countdata->scatter, the ln(determinant) is in countdata->scatter_det
	// 		and the ln of the relevant multiariate gamma is in scatter_gamma
	//
	// density is ( [n-p-1]/2 * scatter_det - [1/2] trace [sigma^-1 * scatter] - [np/2] ln(2) - [n/2] ln(det(sigma)) - scatter_gamma

	double toreturn = 0;
	int s;
	double ld;
	int p = countdata->npop;
	int n = countdata->nsnp -1;

	//copy the covariance matrix over, declare inverse
	gsl_matrix * work = gsl_matrix_alloc(p,p);
	gsl_matrix_memcpy( work, sigma );
	gsl_permutation * perm = gsl_permutation_alloc(p);
	gsl_matrix * inv  = gsl_matrix_alloc(p, p);
	gsl_matrix * ViU = gsl_matrix_alloc(p, p);

	//do LU decomposition
	gsl_linalg_LU_decomp( work, perm, &s );

	//invert sigma
	gsl_linalg_LU_invert( work, perm, inv );

	//get log of determinant
	ld = gsl_linalg_LU_lndet( work );

	//multiply inverse of cov by scatter, get trace
	gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, inv, countdata->scatter, 0.0, ViU);
	double trace = 0;
	for (int i = 0; i < countdata->npop; i++) trace+= gsl_matrix_get(ViU, i, i);

	toreturn+= ( (double) n- (double) p-1.0 )/2.0 * countdata->scatter_det - trace/2.0;
	toreturn+= -( (double) n* (double) p/2.0) * log(2.0);
	toreturn += -((double) n/2.0)*ld;
	toreturn+= -countdata->scatter_gamma;
	return toreturn;
}
