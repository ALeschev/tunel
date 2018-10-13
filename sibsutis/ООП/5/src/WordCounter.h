#include <map>
#include <set>
using namespace std;

class WordCounter
{
	map<string, int> wordCount;
	set<string> dict;

public:
	WordCounter();

	void ReadDictionary(string );
	void ReadSource(string );
	void WriteStatistics(string );
	
private:
	string toLowerCase(string );
	string removingPunctuation(string );
};