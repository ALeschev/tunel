#include "ptr.h"

using namespace std;

int main(void)
{
	array<int> arr(5);

	for (array<int>::iterator iter = arr.begin(); iter != arr.end(); ++iter)
		cout << *iter << std::endl;

	for (array<int>::iterator iter = arr.begin(); iter != arr.end(); ++iter)
		*iter = 5;

	for (array<int>::iterator iter = arr.begin(); iter != arr.end(); ++iter)
		std::cout << *iter << std::endl;

	std::copy(arr.begin(), arr.end(), std::ostream_iterator<int>(std::cout, "\n"));

	return 0;
}
