#include "dot.h"

Dot::Dot(int i, int j) {
	
	_i      = i;
	_j      = j;
	_value  = 0;
	_isBomb = false;
	_isOpen = false;
	_isFlag = false;
}

Dot::Dot(const Dot &object) {

	_i      = object._i;
	_j      = object._j;
	_value  = object._value;
	_isBomb = object._isBomb;
	_isOpen = object._isOpen;
	_isFlag = object._isFlag;
}

int Dot::i() {
	return _i;
}

int Dot::j() {
	return _j;
}

void Dot::setBomb() {
	_isBomb = true;
}

bool Dot::isBomb() {
	return _isBomb;
}

void Dot::incValue() {
	_value++;
}

int Dot::value() {
	return _value;
}

void Dot::setOpen() {
	_isOpen = true;
}

bool Dot::isOpen() {
	return _isOpen;
}

void Dot::setFlag(bool flag) {
	_isFlag = flag;
}

bool Dot::isFlag() {
	return _isFlag;
}