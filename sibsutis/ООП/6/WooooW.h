#include <iostream>
#include <map>

using namespace std;

class WooooW
{
	map<string, string> dict;

	string toLowerCase(string );
	string removingPunctuation(string , char *);
public:
	WooooW();
	void ReadDictionary(string );
	void RewriteSource(string );
};