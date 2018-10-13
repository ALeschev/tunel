#include "intarray.h"
#include <iostream>

int IntArray::count_class = 0;

IntArray::IntArray(int size)
{
	_size = size;
	_array = new int[_size];
	count_class++;
	for(int i=0; i<_size; i++)
		_array[i] = 0;
}

IntArray::IntArray(const IntArray& object){
	_size = object._size;
	_array = new int[_size];
	count_class++;
	for(int i=0; i<_size; i++)
		_array[i] = object._array[i];
}

IntArray::IntArray(int *array, int size){
	_size = size;
	_array = new int[_size];
	count_class++;
	for(int i=0; i<_size; i++)
		_array[i] = array[i];
}

IntArray::~IntArray(){
	delete[] _array;
	count_class--;
}

void IntArray::SumElem() {
	int sum = 0;
	
	for (int i=0; i<_size; i++)
		sum += _array[i];

	std::cout << "Сумма всех элементов: " << sum << std::endl;
}

void IntArray::MultElem() {
	int mult = 1;

	for (int i=0; i<_size; i++)
		mult *= _array[i];

	std::cout << "Произведение всех элементов: " << mult << std::endl;
}

int IntArray::size() {
	return _size;
}

void IntArray::print() {
	for(int i=0; i<_size; i++)
		std::cout << _array[i] << " ";
	std::cout << std::endl;
}

int IntArray::count_obj() {
	return count_class;
}
