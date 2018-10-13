#include <istream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "WordCounter.h"
#include <algorithm>

using namespace std;

WordCounter::WordCounter() {
}

void WordCounter::ReadDictionary(string fileName) {

	ifstream dictionary (fileName.c_str());

	string word;
	if (!dictionary) {
		cout << "Cannot open source file" << endl;
		return;
	} else {
		while (getline(dictionary, word, '\n')) {
//			word = removingPunctuation(word);
			word = toLowerCase(word);
			dict.insert(word);
		}
	}
}

void WordCounter::ReadSource(string fileName) {

	ifstream inputFile (fileName.c_str());

	string word;
	if (!inputFile) {
		cout << "Cannot open source file" << endl;
		return;
	} else {
		while (getline(inputFile, word, ' ')) {
			word = removingPunctuation(word);
			word = toLowerCase(word);
			if (!dict.count(word)) {
				if (wordCount.count(word)) 
					wordCount[word]++;
				else 
					wordCount.insert(pair<string, int>(word, 1));
			}
		}
	}
}

void WordCounter::WriteStatistics(string fileName) {

	map<string,int>:: iterator i;
	for(i = wordCount.begin(); i != wordCount.end(); i++) {
		if (i->first.size() > 3)
			cout << i->first << ":" << i->second << endl;
	}
}

string WordCounter::toLowerCase(string word) {

	transform(word.begin(), word.end(), word.begin(), ::tolower);
	return word;
}

string WordCounter::removingPunctuation(string word) {

	char last = word.at(word.size() - 1);
	if (last == '.' || last == ',' || last == '?' || last == '!')
		word.at(word.size() - 1) = '\0';
	return word;
}