#include "WooooW.h"

int main(void)
{
	WooooW *test = new WooooW();

	test->ReadDictionary("dictionary");
	test->RewriteSource("input");

	return 0;
}