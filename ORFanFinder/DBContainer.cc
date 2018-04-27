#include "DBContainer.h"

#include <fstream>
#include <iostream>

using std::ifstream;
using std::cerr;
using std::endl;
using std::ios;

void DBContainer::init(const char* dbFile){
	avail = true;

	// Get size of database
	ifstream file(dbFile, ios::in|ios::binary|ios::ate);
	dbSize = ((int)file.tellg()-1)/2;
	file.close();

	// Read database
	file.open(dbFile, ios::in|ios::binary);
	if(file.good()){
		// Get the size of the checksum
		file.read(&checkBits, 1);
		taxBits = sizeof(int)*8 - checkBits;

		// Read the seeds table
		seeds.resize(dbSize/sizeof(int));
		int* seedArray = seeds.data();
		file.read((char*)seedArray, dbSize);

		// Read the values table
		values.resize(dbSize/sizeof(int));
                int* valArray = values.data();
                file.read((char*)valArray, dbSize);

		taxMask = ((1<<taxBits)-1);

		dbSize = dbSize / sizeof(int);
	}else{
		cerr << dbFile << " is invalid." << endl;
		avail = false;
	}
	file.close();
}

DBContainer::DBContainer(const char* dbFile){
        init(dbFile);
}

DBContainer::DBContainer(string& dbFile){
	init(dbFile.c_str());
}

DBContainer::~DBContainer(){


}

unsigned int DBContainer::hashString(unsigned int d, string& key){
	if (d == 0) d = 0x01000193;

	// Use the FNV algorithm
	int size = key.size();
	for(int i = 0; i < size; i++){
		d = ( (d * 0x01000193) ^ key[i] ) & 0xffffffff;
	}
	return d;
}

unsigned int DBContainer::foldHash(unsigned int hash, int size){
        int fold = 16;
        while(size < fold){
                hash = (((hash>>fold) ^ (hash & ((u_int32_t)1<<fold)-1)));
                fold /= 2;
        }
        size = fold*2 - size;
        return (((hash>>size) ^ (hash & ((u_int32_t)1<<size)-1)));
}

int DBContainer::operator[](string& key){
	int result = 0;
	unsigned int hash = hashString(0,key);
	int d = seeds[hash % dbSize];
	if (d < 0) {
		result = values[-d-1];
	}
	else{
		result = values[hashString(d, key) % values.size()];
	}
	if(foldHash(hash,checkBits) == (unsigned int)result>>taxBits){ // Check for collisions
		return result & taxMask;
	}
	return -1; // The entry in the table was a collision, return -1 to denote not found
}

int DBContainer::operator[](const char* key){
	string str = string(key);
	return operator[](str);
}

bool DBContainer::good(){
	return avail;
}
