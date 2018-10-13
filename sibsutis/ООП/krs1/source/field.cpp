#include "field.h"
#include "dot.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace std;

Field::Field(int bombNum) {

	_bombNumber = bombNum;
	_height     = 8;
	_width      = 8;

	for (int i = 0; i < _height; i++)
		for (int j = 0; j < _width; j++) 
			field[i][j] = new Dot(i, j);

	srand(time(NULL));
	for (int i = 0; i < _bombNumber;) {
		Dot *p = field[rand() % _height][rand() % _width];
		if(p->isBomb()) {
			continue;
		}
		else {
			p->setBomb();
			++i;
		}
	}

	for (int i = 0; i < _height; ++i) {
		for (int j = 0; j < _width; ++j) {
			if (field[i][j]->isBomb()) {
				if (isValidCoordinate(i - 1, j - 1)) 
					field[i - 1][j - 1]->incValue();
				if (isValidCoordinate(i - 1, j)) 
					field[i - 1][j]->incValue();
				if (isValidCoordinate(i - 1, j + 1)) 
					field[i - 1][j + 1]->incValue();
				if (isValidCoordinate(i, j - 1)) 
					field[i][j - 1]->incValue();
				if (isValidCoordinate(i, j + 1)) 
					field[i][j + 1]->incValue();
				if (isValidCoordinate(i + 1, j - 1)) 
					field[i + 1][j - 1]->incValue();
				if (isValidCoordinate(i + 1, j)) 
					field[i + 1][j]->incValue();
				if (isValidCoordinate(i + 1, j + 1)) 
					field[i + 1][j + 1]->incValue();
			}
		}
	}
	_gameIsActive = true;
}

Field::Field(const Field &object) {

	_bombNumber = object._bombNumber;
	_height     = object._height;
	_width      = object._width;

	for (int i = 0; i < _height; i++)
		for (int j = 0; j < _width; j++)
			field[i][j] = object.field[i][j];
}

Dot* Field::getDot(int i, int j) {
	if (isValidCoordinate(i, j))
		return field[i][j];
	else 
		return field[0][0];
}

int Field::getHeight() {
	return _height;
}

int Field::getWidth() {
	return _width;
}

int Field::getBombNumber() {
	return _bombNumber;
}

bool Field::gameIsActive() {
	return _gameIsActive;
}

bool Field::Winner() {
	return _win;
}

void Field::openAll() {

	for (int i = 0; i < _height; i++)
		for (int j = 0; j < _width; j++)
			field[i][j]->setOpen();
}

void Field::openDot(int i, int j) {

	if (isValidCoordinate(i, j) == false)
		return;

	Dot *p = field[i][j];
	
	if (p->isOpen())
		return;
	
	p->setOpen();
	
	if (p->value()) 
		return;
  
	openDot(i - 1, j); 
	openDot(i + 1, j);
	openDot(i, j - 1); 
	openDot(i, j + 1);	
}

void Field::Step(int x, int y, bool flag) {

	if (isValidCoordinate(x, y) == false)
		return;

	field[x][y]->setFlag(flag);

	if (field[x][y]->isOpen())
		return;
	if (field[x][y]->isBomb() && !field[x][y]->isFlag()) {
		_gameIsActive = false;
		_win = false;
		openAll();
		return;
	}
	openDot(x, y);
	if (isWin()) {
		_gameIsActive = false;
		_win = true;
		openAll();
		return;
	}
}

bool Field::isValidCoordinate(int i, int j) {

	return (i >= 0 && j >= 0 && i < _height && j < _width);
}

bool Field::isWin() {

	int emptyDot = _height * _width - _bombNumber;
	for (int i = 0; i < _height; ++i)
		for (int j = 0; j < _width; ++j)
			emptyDot -= field[i][j]->isOpen();

	return (emptyDot == 0);
}

Field::~Field() {
	delete[] field;
}