#include <iostream>
#include "src/WordCounter.h"

using namespace std;

int main(void)
{
	WordCounter Wcounter;

	Wcounter.ReadDictionary("dictionary");
	Wcounter.ReadSource("inputfile");
	Wcounter.WriteStatistics("shit");

	return 0;
}