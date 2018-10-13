#include <iostream>
#include <stdlib.h>

using namespace std;

const int field_size = 4;
const char empty_char = '.';

class Cell
{
private:
	char _cr;
public:
	Cell() { _cr = empty_char; }
	void setChar(char newChar) { _cr = newChar; }
	char getChar() { return _cr; }
};

class Field
{
private:
	Cell *_myField[field_size][field_size];

	bool isValidCoordinate(int i, int j) {
		return ((0 <= i && i < field_size) && (0 <= j && j < field_size));
	}
public:
	Field() {
		for (int i = 0; i < field_size; i++)
			for (int j = 0; j < field_size; j++)
				_myField[i][j] = new Cell();
	}

	Cell* getCell(int i, int j) {
		if (isValidCoordinate(i, j))
			return _myField[i][j];
		else
			return NULL;
	}

	int getSize() { return field_size; }

	void setCharToCell(int i, int j, char val) {
		if (isValidCoordinate(i, j))
			_myField[i][j]->setChar(val);
	}

	char getCharInCell(int i, int j) {
		if (isValidCoordinate(i, j))
			return _myField[i][j]->getChar();
		else 
			return '\0';
	}

	~Field() {
		for (int i = 0; i < field_size; i++)
			for (int j = 0; j < field_size; j++)
				delete _myField[i][j];
		delete[] _myField;
	}

};

class View 
{
public:
	void show(Field *thisField) {
		for (int i = 0; i < field_size; i++) {
			for (int j = 0; j < field_size; j++)
				cout << thisField->getCharInCell(i, j) << ' ';
			cout << endl;
		}
	}
};

class Mediator
{
private:
	Field *_field;
	View  *_viewer;

public:
	Mediator(Field *newField, View *Viewer) {
		_field = newField;
		_viewer = Viewer;
	}
	void ViewField() {
		_viewer->show(_field);
	}
};


class Game
{
private:
	Field *curField;
	Mediator *visual; // ?? o_O
	string validChar;

	bool isValidChar(char val) {
		for (int i = 0; i < validChar.size(); i++)
			if (val == validChar[i])
				return true;
		return false;
	}

	bool isValidCoordinate(int i, int j) {
		return ((0 <= i && i < field_size) && (0 <= j && j < field_size));
	}

	void userGetCoordinate(int *i, int *j) {
		do {
			cout << "Plz enter \'i\' ";
			cin >> *i;
			cout << "Plz enter \'j\' ";
			cin >> *j;
		} while (!isValidCoordinate(*i, *j));
	}

	void userGetChar(char *val) {
		do {
			cout << "Plz enter symbol ";
			cin >> *val;
		} while (!isValidChar(*val));
	}

	bool checkSetChar(int i, int j, int val) {
		for (int k = 0; k < field_size; k++)
			if (curField->getCharInCell(k, j) == val)
				return false;

		for (int k = 0; k < field_size; k++)
			if (curField->getCharInCell(i, k) == val)
				return false;

		return true;
	}

	bool checkEnd() {
		for (int i = 0; i < field_size; i++)
			for (int j = 0; j < field_size; j++)
				if (curField->getCharInCell(i, j) == empty_char)
					return false;

		return true;
	}
public:
	Game() { 
		validChar = "abcd"; 
		curField = new Field(); 
		visual = new Mediator(curField, new View());
	}
	
	void StartGame() {
		int i, j;
		char val;

		cout << "Hello!" << endl << "Field height/width is: 4; For edit: src.h:22" << endl << "Correct symbol is: a, b, c, d; For edit: src.h:149" << endl;
		do {
			userGetCoordinate(&i, &j);
			userGetChar(&val);

			if (checkSetChar(i, j, val))
				curField->setCharToCell(i, j, val);
			else 
				cout << "Error: Char is not unique. Plz try again." << endl;

			visual->ViewField();

		} while (!checkEnd());

		cout << endl << "Congratulation! You can make it! May inglish soy bat" << endl;
	}

	~Game() { 
		delete curField; 
	}

};
