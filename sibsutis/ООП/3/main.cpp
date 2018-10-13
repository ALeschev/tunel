#include "array.h"
#include <vector>

ostream & operator << (ostream& out, const Array<Type>& object){

		for(int i = 0; i < object._size; i++){
				out <<  object._array[i] << " ";
		}
		out << endl;
		return out;
	}

istream & operator >> (istream& in, const Array<Type>& object) {
		for(int i = 0; i < object._size; i++) {
			in >> object._array[i];
			if(in.fail()) {
			   cerr << "Error input data" << endl;
			   in.clear();
			   return in;
			}
		}
		return in;
	}


int main(void)
{
	int ar[10] = {1, 34, 2, 2, 1, 12, 1, 345, 1, 2};
	Array<int> *test = new Array<int>(ar, 10);
	cout << *test;
	test->MinElem();
	test->MaxElem();
	test->push_back(123);
	cout << *test;
	test->pop_back();
	cout << *test;

//	try {
		Array<int> *test2 = new Array<int>(1000001);
//	} catch(ArrayException){
		cerr<<"ArrayException"<<endl;
///	}

	cin >> *test;
	cout << *test;
	

  	test->~Array();

	return 0;
}

