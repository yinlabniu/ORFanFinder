#ifndef TAXTREE_H

#define TAXTREE_H


#include "TaxNode.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class TaxTree{
private:
	TaxNode* head;
	unordered_map<int, TaxNode*> nodes;
	bool avail;
public:
	static unordered_set<int> filteredNodes;

	TaxTree(){}
	TaxTree(string);
	~TaxTree();

	TaxNode* getHead();
	TaxNode* getChild(int);
	bool good();
};

#endif
