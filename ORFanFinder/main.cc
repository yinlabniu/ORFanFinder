#include "DBContainer.h"
#include "TaxTree.h"
#include "OrfProcessor.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <regex>
#include <set>

#include <time.h>
#include <unordered_set>
#include <cstring>

using namespace std;

void printHelp(int help){
        if(!help) cerr << "Incorrect number of command line arguments" << endl;
        cerr << "Usage: ORFanFinder <-query BLAST_Output_filename> <-id ID_list_filename>" << endl
			<< "\t\t<-tax taxonomy_id> <-nodes nodes_filename>" << endl
			<< "\t\t<-db database_filename> <-out output_filename>" <<endl
			<< "\t\t[-names taxonomy_to_names_filename] [-threads int_value]" << endl;
	if(help <2)cerr << "Use --help for more detailed information." << endl;
	if(help == 2){
		cerr << endl
		     << "<-query BLAST_Output_filename>: The resulting tabular file from a BLAST where the genme was run against the database, such as nr." << endl
		     << "<-id ID_list_filename>: A list of every ID that was used in the FASTA file to run the BLAST." << endl
		     << "<-tax taxonomy_id>: The Taxnomy ID of the genome being queried." << endl
		     << "<-nodes nodes_filename>: Lists the taxonomy ID, the taxonomy ID of its parent, and the taxonomy rank in a tab-delimited file." << endl
		     << "<-db database_filename>: Database file generated from gene ID to taxonomy ID map using the formatDatabase tool." << endl
		     << "<-out output_filename>: Path to the file where output will be written." << endl
		     << "[-names taxonomy_to_names_filename]: Lists the Taxonomy ID and the name of that rank in a tab-delimited file. If this is provided, extra details are shown." << endl
		     << "[-threads int_value]: Number of worker threads to use. Total threads will be this number add three. Default 1." << endl;

	}
}

int main(int argc, char* argv[]){
	bool showDetails=false;

	map<string,string> arguments;
	for(int i=1;i<argc;i+=2){
		if(strcmp(argv[i],"-h")==0){
				printHelp(1);
				return 0;
		}
		else if(strcmp(argv[i],"--help")==0){
			printHelp(2);
                        return 0;
		}
		else {
			if(argv[i][0] != '-'){
				cerr << "Invalid argument " << argv[i] << endl;
				printHelp(1);
				return 0;
			}
			else{
				arguments[argv[i]] = argv[i+1];
			}
		}
    }


	if(arguments["-query"] == ""){
		cerr << "A BLAST file must be provided.";
		printHelp(1);
		return 0;
	}
	if(arguments["-id"] == ""){
                cerr << "An ID file must be provided.";
                printHelp(1);
                return 0;
        }
	if(arguments["-tax"] == ""){
                cerr << "A taxonomy ID must be provided.";
                printHelp(1);
                return 0;
        }
	if(arguments["-nodes"] == ""){
                cerr << "A nodes file must be provided.";
                printHelp(1);
                return 0;
        }
	if(arguments["-db"] == ""){
		cerr << "A database file must be provided.";
		printHelp(1);
		return 0;
	}

	if(arguments["-out"] == ""){
		cerr << "An output file must be provided.";
		printHelp(1);
		return 0;
	}

	if(arguments["-names"] != ""){
		showDetails = true;
    }


	ifstream file;
	string line;
	string item1, item2, item3;

	file.open(arguments["-query"],ifstream::in);
        if(!file.is_open()){ cerr << "BLAST file is invalid."; return 0;}
	file.close();

	unordered_set<string> hits;
	file.open(arguments["-id"],ifstream::in);
        if(!file.is_open()){cerr << "ID list file is invalid."<<endl; return 0;}
	file.close();

	// create a taxonomy tree
	TaxTree taxTree = TaxTree(arguments["-nodes"]);
	if(!taxTree.good()){
                cerr << "Loading taxonomy tree failed." << endl;
                return 0;
        }

	unordered_map<string,string> names;

	file.open(arguments["-names"],ifstream::in);
	if(!file.is_open() && showDetails){cerr << "Names file is invalid."<<endl; return 0;}
	if(file.is_open()){
	        while(getline(file,item1,'\t')){
	                getline(file,item2,'\n');

	                names[item1] = item2;
	        }
	        file.close();
	}

	DBContainer db(arguments["-db"]);
	if(!db.good()){
		cerr << "Loading database failed." << endl;
		return 0;
	}

	int threads = 1;
	if(arguments["-threads"] != ""){
		threads = stoi(arguments["-threads"]);
	}
	OrfProcessor orf(arguments["-query"], arguments["-out"], arguments["-tax"], arguments["-id"], taxTree, db, names, showDetails, threads+2);
	orf.join();

}


