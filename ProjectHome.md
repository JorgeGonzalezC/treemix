# This repository has moved to [BitBucket](https://bitbucket.org/nygcresearch/treemix/wiki/Home) and is no longer being maintained here #

_TreeMix_ is a method for inferring the patterns of population splits and mixtures in the history of a set of populations. In the underlying model, the modern-day populations in a species are related to a common ancestor via a graph of ancestral populations. We use the allele frequencies in the modern populations to infer the structure of this graph.

The details of the  _TreeMix_ model are presented in:<br>
Pickrell JK and Pritchard JK. <a href='http://www.plosgenetics.org/article/info%3Adoi%2F10.1371%2Fjournal.pgen.1002967'>Inference of population splits and mixtures from genome-wide allele frequency data</a>.<br>
<br>
<br>
Some extensions are presented in:<br>
Pickrell JK, Patterson N, Barbieri C, Berthold F, Gerlach L, Güldemann T, Kure B, Mpoloka SW, Nakagawa H, Naumann C, Lipson M, Loh PR, Lachance J, Mountain J, Bustamante CD, Berger B, Tishkoff SA, Henn BM, Stoneking M, Reich D, Pakendorf B. <a href='http://www.ncbi.nlm.nih.gov/pubmed/23072811'>The genetic prehistory of southern Africa</a>.<br>
<br>
We describe an application of this model to looking for natural selection in humans and dogs at <a href='http://www.genomesunzipped.org/2012/03/identifying-targets-of-natural-selection-in-human-and-dog-evolution.php'>Genomes Unzipped</a>.<br>
<br>
<br>
<h1>What's new:</h1>

<h3>6/5/13:</h3>
TreeMix 1.12 released.<br>
<ul><li>Fixes a bug that caused the reported relative likelihoods to be incomparable between trees and graphs. Many thanks to Mait Metspalu and Mike DeGiorgio for working through this.<br>
</li><li>Also adds a -seed option for setting the random seed from the command line</li></ul>

<h3>11/20/12:</h3>
The <i>TreeMix</i> paper has been published in PLoS Genetics: <br>
Pickrell JK and Pritchard JK <a href='http://www.plosgenetics.org/article/info%3Adoi%2F10.1371%2Fjournal.pgen.1002967'>Inference of population splits and mixtures from genome-wide allele frequency data</a>

<h3>10/22/12:</h3>
Release of version 1.11.<br>
<ul><li>Fixes a bug that sometimes caused crashes when using microsatellite data</li></ul>

<h3>10/1/12:</h3>
Release of version 1.1.<br>
<ul><li>Allows input of microsatellite data. For a description of the microsatellite model, see <a href='https://code.google.com/p/treemix/downloads/detail?name=microsat_model.pdf'>here (pdf)</a></li></ul>

<ul><li>Allows incorporation of known migration events</li></ul>

<ul><li>Small other bug fixes</li></ul>

<h3>7/25/12:</h3>
Preprint: <a href='http://arxiv.org/abs/1207.5552'>"The genetic prehistory of southern Africa"</a> is available on arXiv. The new features in <i>TreeMix</i> described in this preprint will be available in the next release (estimated Sept. 2012).<br>
<br>
<h3>5/24/12:</h3>

Release of version 1.04.<br>
<ul><li>Forces migration edges to have weight less than 0.5</li></ul>

<ul><li>Include three- and four- population tests for treeness from <a href='http://www.nature.com/nature/journal/v461/n7263/abs/nature08365.html'>Reich et al. 2009</a> (programs are called threepop and fourpop, respectively)</li></ul>

To run threepop or fourpop, the input is standard TreeMix input. Then run (e.g.)<br>
<br>
<code>&gt;threepop -i input.gz -k 500</code>

This will print f3 statistics for all populations to stdout, and calculate standard errors in blocks of 500 SNPs. For example, running this on the test input files will give a set of output like:<br>
<br>
<code>Estimating f_3 in 59 blocks of size 500</code>

<code>total_nsnp 29999 nsnp 29999</code>

<code>Dai;Han,Sardinian 0.00112445 0.000276542 4.06609</code>

<code>Han;Sardinian,Dai 0.000536062 0.000211323 2.53669</code>

<code>Sardinian;Han,Dai 0.0289054 0.000867602 33.3165</code>

The line <code>Sardinian;Han,Dai 0.0289054 0.000867602 33.3165</code> tells you that f3(Sardinian;Han,Dai) is ~0.03, with a standard error of 0.0009, which corresponds to a z-score of 33. For information on how to interpret these tests, see Reich et al. (2009).<br>
<br>
<h3>3/12/12:</h3>

Added a small script to convert stratified allele frequencies output from plink into TreeMix format. This will be incorporated into the next release, but for the moment must be downloaded separately. To run this, let's say you have data in plink format (e.g., data.bed, data.bim, data.fam) and a plink cluster file matching each individual to a population (data.clust).<br>
<br>
Now you  run:<br>
<br>
<code>&gt;plink --bfile data --freq --missing --within data.clust</code>

<code>&gt;gzip plink.frq</code>

<code>&gt;plink2treemix.py plink.frq.gz treemix.frq.gz</code>

The file treemix.frq.gz can now be used as input for TreeMix.<br>
<br>
<br>
Version 1.0.3:<br>
<ul><li>small bug fixes</li></ul>

Version 1.0.2:<br>
<ul><li>removed an unnecessary header that sometimes caused compilation problems<br>
</li><li>small big fixes</li></ul>

Version 1.0.1:<br>
<ul><li>this is the first major release