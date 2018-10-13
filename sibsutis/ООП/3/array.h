#include"arrayexception.h"
#include<iostream>
#include<sstream>
#include<typeinfo>
#include<cstdlib>

#ifndef ARRAY_H
#define ARRAY_H

using namespace std;


template <class Type>
class Array
{

public:
	Array(int size = 10) {
	initial(size);
	for(int i = 0; i < _size; i++) {
		_array[i] = 0;
	}
}

	Array(Type *array, int size) {
		initial(size);
		for (int i = 0; i < _size; i++) {
			_array[i] = array[i];
		}
	}

	Array(const Array<Type> &object) {
		initial(object._size);
		for(int i = 0; i < _size; i++) {
			_array[i] = object._array[i];
		}
	}

	~Array() {
		delete[] _array;
	}

	int size() {
		return _size;
	}

	void MinElem() {
		int min = _array[0];
		
		for (int i=0; i<_size; i++)
			if(min > _array[i])
				min = _array[i];

		cout << "Минимальный элемент: " << min << endl;
	}

	void MaxElem() {
		int max = 0;

		for (int i=0; i<_size; i++)
			if(max < _array[i])
				max = _array[i];

		cout << "Максимальный элемент: " << max << endl;
	}

	void push_back(int elem) {
		Type *tmpArray = _array;
		delete[] _array;
		_size += 1;
		initial(_size);
		_array = tmpArray;
		_array[_size-1] = elem;
	}

	void pop_back() {
		Type *tmpArray = _array;
		delete[] _array;
		_size -= 1;
		initial(_size);
		for (int i=0; i<_size; i++)
			_array[i] = tmpArray[i];
	}

	bool operator == (const Array<Type> &object) {
		if(_size != object._size) 
			return false;
		for(int i = 0; i < _size; i++) {
			if(_array[i] != object._array[i])
				return false;
		}
		return true;
	}


	Array<Type>& operator = (const Array<Type> &object) {

		if(_size < object._size){
			delete[] _array;
			_size = object._size;
			_array = new int[_size];
		}

		for(int i = 0; i < _size; i++) {
			_array[i] = object._array[i];
		}
		return *this;
	}

	Array<Type>& operator = (const int value) {

		for(int i = 0; i < _size; i++) {
			_array[i] = value;
		}
		return *this;
	}


	bool operator != (const Array<Type> &object) {
		if(_size != object._size) 
			return true;
		for(int i = 0; i < _size; i++){
			if(_array[i] == object._array[i])
				return false;
		}
		return true;
	}

	Type & operator [] (const int index) {
		if(index > (_size-1)) 
			throw ArrayException();
		cout << _array[index];
		return _array[index];
	}

private:
	Type *_array;
	int _size;
	void initial(int size) {
		if(size > 1000000) {
			throw ArrayException();
		}

		_size = size;
		try {
			_array=new Type[_size];
		} catch(bad_alloc) {
			cerr << "Error: Cannot create new array." << endl;
		}
	}
};


#endif // ARRAY_H
