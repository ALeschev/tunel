#include "StrArray"
#include <iostream>

StrArray::StrArray() {
	_array = NULL;
	count_class++;
}
StrArray::StrArray(const StrArray& object) {

}

StrArray::~StrArray() {
	delete[] _array;
	count_class--;
}
