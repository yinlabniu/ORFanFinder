#include "TaxNode.h"

#include <iostream>
#include <algorithm>

/* TaxNode
 *
 * Constructor
 *
 * @args int, tax id of node
 */
TaxNode::TaxNode(int nID, string nRank): id(nID), rank(nRank){}


/* ~TaxNode
 *
 * Deconstructor
 *
 */
TaxNode::~TaxNode(){
/*	for(auto itr = children.begin(); itr != children.end(); itr++){
		return;
		if(*itr != NULL && *itr != this) delete (*itr);
	}
	children.clear();
*/
}

/* setParent
 *
 * Set parent node
 *
 * @args Node*, the parent node
 */
void TaxNode::setParent(TaxNode* newParent){
	parent = newParent;
}

/* getParent
 *
 * Gets the parent node
 *
 * @return TaxNode*, the parent node
 */
TaxNode* TaxNode::getParent(){
	return parent;
}


int TaxNode::getID(){
	return id;
}


string TaxNode::getRank(){
	std::lock_guard<mutex> guard(lock);
	return rank;
}

void TaxNode::setRank(string newRank){
	std::lock_guard<mutex> guard(lock);
	rank = newRank;
}

/* addChild
 *
 * Add a child node
 *
 * @args Node*, the new child node
 */
void TaxNode::addChild(TaxNode* newChild){
	children.push_back(newChild);
}

/* childNum
 *
 * gets the index of a child
 *
 * @args TaxNode*, pointer to child
 * @return int, index of child
 */
int TaxNode::childNum(TaxNode* child){
	return distance(children.begin(), find(children.begin(),children.end(),child));
}
