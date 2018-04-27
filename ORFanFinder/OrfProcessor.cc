#include "OrfProcessor.h"

#include <iostream>
#include <fstream>
#include <algorithm>

using std::cout; using std::cerr; using std::endl;
using std::ifstream; using std::ofstream;
using std::ios;


OrfProcessor::OrfProcessor(string inputFile, string outputFile, string taxId, string idFile, TaxTree& rTaxTree, DBContainer& rDb, unordered_map<string,string>& rNames, bool showDetails, int maxThreads)
			:taxTree(rTaxTree), db(rDb), names(rNames){
	this->queryId = stoi(taxId);
	this->showDetails = showDetails;
	this->idFile = idFile;

	//hash the tax ids of each rank for the query protein
	TaxNode* tax = taxTree.getChild(queryId);
	if(tax == NULL){
		cerr << "Error: Taxon " << taxId << " does not exist. (Check that the provided tax ID exists in the nodes file provided.)" << endl;
		exit(0);
	}

	// Climb the taxonomy tree and note lineage
	// This is done outside of threads because it is needed across threads
	TaxNode* next = tax->getParent();
	while(next != tax){
		if(next == NULL){
			cerr << "Error: Taxon " << tax->getID() << " does not have an existing parent. (Check the provided tax ID and the provided nodes file.)" << endl;
			exit(0);
		}
		if(tax->getRank() == "no rank" && next->getRank() == "species"){
				tax->setRank("subspecies");
		}
		queryTax[tax->getRank()] = tax->getID();
		if(tax->getRank() != "no rank")
			levels.insert(levels.begin(), tax->getRank());
		tax = next;
		next = tax->getParent();
	}

	isComplete = false;

	// Create one thread for reading, one for writing, and n for processing.
	threadsVec.push_back( thread(&OrfProcessor::getInput, this, inputFile, maxThreads*3) );
	threadsVec.push_back( thread(&OrfProcessor::writeOutput, this, outputFile) );
	maxThreads = max(maxThreads, 3) - 2;
	threads = maxThreads + 2;
	for(int i = 0; i < maxThreads; i++){
		threadsVec.push_back( thread(&OrfProcessor::determineOrfans, this) );
	}
}

OrfProcessor::~OrfProcessor(){

}

void OrfProcessor::getInput(string inputFile, int queueSize){
	ifstream file(inputFile);
	string input1, input2;
	string currentGene;
	vector<string> inputBlock;

	// Get non-comment start of input
	getline(file,input1,'\t');
	while(input1[0] == '#'){
		file.ignore(1000, '\n');
                getline(file, input1, '\t');
        }
	input1 = input1.substr(0,input1.find(" "));
	getline(file,input2,'\t');
	file.ignore(1000, '\n');
	inputBlock.push_back( input1 );
	if(db[input2] != queryId){
		inputBlock.push_back( input2 );
	}
	currentGene = input1;

	while(getline(file,input1,'\t')){
		while(input1[0] == '#'){
			file.ignore(1000, '\n');
	                getline(file, input1, '\t');
	        }
		input1 = input1.substr(0,input1.find(" "));
		getline(file,input2,'\t');
		file.ignore(1000, '\n');

		if(input1 != currentGene){
			while(getInputQueueSize() > queueSize){
				std::this_thread::sleep_for (std::chrono::milliseconds(1));
			}
			hits.insert(currentGene);
			writeInputQueue(inputBlock);
			inputBlock.clear();
			inputBlock.push_back(input1);
			currentGene = input1;
		}

		if(db[input2] != queryId)
			inputBlock.push_back( input2 );
	}

	hits.insert(currentGene);
	writeInputQueue(inputBlock);
	inputBlock.clear();
	inputBlock.push_back(input1);
	currentGene = input1;

	isComplete = true;
	decrementThreads();
	file.close();
}

void OrfProcessor::writeOutput(string outputFile){
	string output;
	ofstream file(outputFile, ios::out);
	int size = 0;

	// Poll the write queue
	while(true){
		// Exit if no more processors are alive and queue is empty
		size = getOutputQueueSize();
		if( threads < 2 && !size){
			break;
		}

		// Nothing to write
		else if(!size){
			std::this_thread::sleep_for (std::chrono::milliseconds(3));
			continue;
		}

		// Write the chunk
		output = getOutputQueue();
		file << output << endl;

		std::this_thread::sleep_for (std::chrono::milliseconds(3));
	}

	// Genes not found are strict ORFans
	ifstream ids(idFile, ios::in);
	while(getline(ids,output,'\n')){
		if(hits.find(output) == hits.end())
			file << output << "\tstrict ORFan" << endl;
	}
	ids.close();

	// cleanup
	decrementThreads();
	file.close();
}

void OrfProcessor::determineOrfans(){
	TaxNode* tax;
	string prevName;
	int prev;

	// These contain the hits for each rank
	vector<pair<string,unordered_set<int>* > > ranks;
	unordered_map<string,unordered_set<int>* > Iranks;

	for(auto itr = levels.begin(); itr != levels.end();  itr++){
		unordered_set<int>* taxon = new unordered_set<int>();
		ranks.push_back(make_pair(*itr,taxon));
		Iranks.insert(make_pair(*itr,taxon));
	}
	bool stopProcessing = false;

	while(!isComplete || getInputQueueSize() > 0){
		vector<string> input;
		while(true){
			inputMutex.lock();
			if(inputQueue.size() > 0){
				input = inputQueue.front();
				inputQueue.pop_front();
				inputMutex.unlock();
				break;
			}
			else if(isComplete){
				inputMutex.unlock();
				stopProcessing = true;
				break;
			}
			inputMutex.unlock();
			std::this_thread::sleep_for (std::chrono::milliseconds(3));
		}

		if(stopProcessing) break;

		auto end = input.end();
		string gene = input[0];

		if(input.size() == 1){
			writeOutputQueue(gene + "\tstrict ORFan");
		}
		else{
			for(auto itr = input.begin()+1; itr != end; itr++){
				prev = db[*itr];
				if(prev <= 0)
					continue;
				tax = taxTree.getChild(prev);
				if(tax == NULL){
					cerr << "Error: taxonomy ID " << prev << " was not found in database." << endl;
					continue;
				}

				TaxNode* next = tax->getParent();
				while(next != tax){
					if(tax->getRank() == "no rank" && next->getRank() == "species"){
						tax->setRank("subspecies");
					}
					unordered_set<int>* set = Iranks[tax->getRank()];
					if(set){
						set->insert(tax->getID());
					}

					tax = next;
					next = tax->getParent();
				}

			}
			prevName = "native gene";
			string details = "";
			auto itr3 = ranks.begin();
			for(; itr3 != ranks.end(); itr3++){
				auto taxonFound = itr3->second->find(queryTax[itr3->first]);
				if(( taxonFound != itr3->second->end() && itr3->second->size() > 1) || (
							itr3 == ranks.end()-1 && itr3->second->size() > 0) ){
					if(prevName!="native gene"){
						details +=  gene + "\t" + prevName + " ORFan";
						if(showDetails){
							details +=  " - ";
							details +=  names[to_string((long long int)queryTax[prevName])];
						}
					}else{
						details +=  gene + "\t" + prevName;
					}
					if(showDetails) details += printData(itr3, ranks);

					break;
				}
				else if(taxonFound == itr3->second->end()){
					break;
				}
				if(queryTax[itr3->first] != 0) prevName = itr3->first;
			}
			if(details != ""){
				writeOutputQueue(details);
			}
		}

		for( auto itr2 = ranks.begin(); itr2 != ranks.end(); itr2++){
			unordered_set<int>* vec = itr2->second;
			vec->clear();
		}

		std::this_thread::sleep_for (std::chrono::milliseconds(1));
	}

	for(auto itr = ranks.begin(); itr != ranks.end();  itr++){
		delete itr->second;
	}

	decrementThreads();
}

void OrfProcessor::writeOutputQueue(string output){
	std::lock_guard<mutex> lock(outputMutex);
	outputQueue.push_back(output);
}

string OrfProcessor::getOutputQueue(){
	std::lock_guard<mutex> lock(outputMutex);
	string out = outputQueue.front();
	outputQueue.pop_front();
	return out;
}

void OrfProcessor::writeInputQueue(vector<string>& input){
	std::lock_guard<mutex> lock(inputMutex);
	inputQueue.push_back(input);
}

vector<string> OrfProcessor::getInputQueue(){
	std::lock_guard<mutex> lock(inputMutex);
	vector<string> input = inputQueue.front();
	inputQueue.pop_front();
	return input;
}

int OrfProcessor::getInputQueueSize(){
	std::lock_guard<mutex> lock(inputMutex);
	return inputQueue.size();
}

int OrfProcessor::getOutputQueueSize(){
	std::lock_guard<mutex> lock(outputMutex);
	return outputQueue.size();
}

void OrfProcessor::decrementThreads(){
	std::lock_guard<mutex> lock(threadMutex);
	threads--;
}

string OrfProcessor::printData(vector<pair< string,unordered_set<int>* > >::iterator itr, vector<pair< string,unordered_set<int>* > >& vec){
	string total = "";
	total += "\t";
        vector<pair< string,unordered_set<int>* > >::iterator rItr(itr);
        for(;rItr!=vec.end(); rItr++){
                total += "[" + rItr->first + "," + to_string((long long int)rItr->second->size()) + "]";
                for(auto i = rItr->second->begin(); i != rItr->second->end(); i++){
			total += names[ to_string((long long int)*i) ] + "(" + names[to_string((long long int)taxTree.getChild(*i)->getParent()->getID())]  + "),";
                }
		total += "\t";
        }
	return total;
}


void OrfProcessor::join(){
	for(auto itr = threadsVec.begin(); itr != threadsVec.end(); itr++){
		itr->join();
	}
}

void OrfProcessor::detach(){
	for(auto itr = threadsVec.begin(); itr != threadsVec.end(); itr++){
		itr->detach();
	}
}
