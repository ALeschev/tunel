#include <iostream>
#include"intarray.h"

#define SIZE  4
using namespace std;

int main()
{
	int array[SIZE] = {1, 6, 3, 5};
	int count = 0;
	IntArray *obj1 = new IntArray(array, 4);
	obj1->print();
	obj1->SumElem();
	obj1->MultElem();

	IntArray obj4;
	obj4.print();

	IntArray *obj2 = new IntArray();
	cout << "Size:" << obj2->size() << endl;

	cout << "Колличество созданых объектов: " << obj1->count_obj() << endl;

	return 0;
}

