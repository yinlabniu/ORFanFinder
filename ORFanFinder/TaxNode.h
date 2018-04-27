#ifndef TAXNODE_H

#define TAXNODE_H

#include <vector>
#include <string>
#include <mutex>

using namespace std;

class TaxNode{
private:
	vector<TaxNode*> children;
	TaxNode* parent;
	int id;
	string rank;
	mutex lock;
public:
	TaxNode(int,string);
	~TaxNode();

	void setParent(TaxNode*);
	TaxNode* getParent();
	int getID();
	string getRank();
	void setRank(string);

	void addChild(TaxNode*);
	int childNum(TaxNode*);
};

#endif
