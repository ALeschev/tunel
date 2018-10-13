#include "WooooW.h"
#include <istream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

WooooW::WooooW(){}

void WooooW::ReadDictionary(string filename) {

	ifstream dictionary (filename.c_str());

	string key, word;
	if (!dictionary) {
		cout << "Cannot open source file" << endl;
		return;
	} else {
		while (getline(dictionary, key, ' ')) {
			key = toLowerCase(key);
			getline(dictionary, word, '\n');
//			word = toLowerCase(word);
			dict.insert(pair<string, string>(key, word));
		}
	}
	dictionary.close();
}

void WooooW::RewriteSource(string filename) {

	ifstream inputFile (filename.c_str());
	ofstream outputFile ("output", ios::app);

	string word;
	if (!inputFile) {
		cout << "Cannot open source file" << endl;
		return;
	} else {
		while (getline(inputFile, word, ' ')) {
			char punc = ' ';
			word = removingPunctuation(word, &punc);
//			word = toLowerCase(word);

			cout << word << endl;

			if (dict.count(word)) {
				outputFile << dict[word] << punc;
			}
			else {
				outputFile << word << ' ';
			}
		}
	}

	inputFile.close();
	outputFile.close();
}

string WooooW::toLowerCase(string word) {
	transform(word.begin(), word.end(), word.begin(), ::tolower);
	return word;
}

string WooooW::removingPunctuation(string word, char *removing) {
	char last = word.at(word.size() - 1);

	cout << word << endl;

	if (last == '.' || last == ',' || last == '?' || last == '!') {
		*removing = word.at(word.size() - 1);
		word.erase(word.end() - 1);
	}
	return word;
}