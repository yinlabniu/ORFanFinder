#ifndef _ORF_PROCESSOR_H_
#define _ORF_PROCESSOR_H_

#include "TaxTree.h"
#include "DBContainer.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <thread>
#include <mutex>

using std::unordered_map; using std::unordered_set;
using std::vector;
using std::list; 

using std::thread; using std::mutex;

class OrfProcessor{
private:
	TaxTree& taxTree;
	DBContainer& db;
	int queryId;
    bool showDetails;
	string idFile;
	bool isComplete;
	
	vector<string> levels;
	unordered_map<string, int> queryTax;
	unordered_map<string, string>& names;
	
	list<vector<string>> inputQueue;
	list<string> outputQueue;
	unordered_set<string> hits;

	mutex inputMutex;
	mutex outputMutex;
	mutex threadMutex;
	vector<thread> threadsVec;
	int threads;

	void getInput(string, int);
	void writeOutput(string);
	void determineOrfans();
	string printData(vector<pair< string,unordered_set<int>* > >::iterator itr, vector<pair< string,unordered_set<int>* > >& vec);

	void writeOutputQueue(string);
	string getOutputQueue();
	int getInputQueueSize();
	void writeInputQueue(vector<string>&);
	vector<string> getInputQueue();
	int getOutputQueueSize();
	void decrementThreads();
public:
	OrfProcessor(string inputFile, string outputFile, string taxId, string idFile, TaxTree& rTaxTree, DBContainer& rDb, unordered_map<string,string>&, bool showDetails, int maxThreads);
	~OrfProcessor();

	void join();
	void detach();
};


#endif // _ORF_PROCESSOR_H_
