#include "TaxTree.h"
#include <fstream>
#include <unordered_map>
#include <vector>

#include <iostream>


unordered_set<int> TaxTree::filteredNodes;


/*
 * TaxTree
 *
 * Constructor
 *
 * @args string, filename of nodes file
 */
TaxTree::TaxTree(string fileName){
	ifstream file;
	vector< pair<TaxNode*,int> > tempNodes;
	file.open(fileName.c_str(),ifstream::in);
	avail = true;
	if(!file.good()){
		cerr << "Failure to open nodes." << endl;
		avail = false;
		return;
	}
	string taxID;
	string parentID;
	string rank;

	head = NULL;

	// Read in each node
	while(getline(file,taxID,'\t')){
		getline(file,parentID,'\t');
		getline(file,rank,'\n');
		TaxNode* node = new TaxNode(stoi(taxID), rank);
		if(taxID == "1"){
			head = node;
		}

		tempNodes.push_back( make_pair(node,stoi(parentID)) );
		nodes[stoi(taxID)] = node;
	}

	// Set nodes to their parents and construct tree
	for(auto i = tempNodes.begin(); i != tempNodes.end(); i++){
		TaxNode* parent = nodes[i->second];
                i->first->setParent(parent);
                parent->addChild(i->first);
	}
}

/*
 * ~TaxTree
 *
 * Deconstructor
 */
TaxTree::~TaxTree(){
//	if(head != NULL) delete head;
	for(auto itr = nodes.begin();itr != nodes.end(); itr++){
		delete (itr->second);
	}
}


/* getChild
 *
 * gets the node associated with a given integer
 *
 * @args int, taxonomy id of node
 * @return TaxNode*, the node
 */
TaxNode* TaxTree::getChild(int id){
	return nodes[id];
}


bool TaxTree::good(){
	return avail;
}
