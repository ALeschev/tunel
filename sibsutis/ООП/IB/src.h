#include <iostream>

using namespace std;

const int size = 8;

class Cell
{
private:
	bool _alive;
public:
	Cell() { _alive = false; }

	bool isAlive()	{ return _alive; }
	void Liven() 	{ _alive = true; }
	void Kill()		{ _alive = false; }
};

class Field
{
private:
	Cell *_field[size][size];
public:
	Field() {
		for (int i = 0; i < size; i++)
			for (int j = 0; j < size; j++)
				_field[i][j] = new Cell();

		 _field[1][1]->Liven();
		 _field[1][2]->Liven();
		 _field[1][3]->Liven();
		 _field[1][4]->Liven();
		 _field[1][5]->Liven();
		 _field[2][2]->Liven();
		 _field[3][2]->Liven();
		 _field[size / 2][size / 2]->Liven();
		 _field[size / 2 + 1][size / 2]->Liven();
		 _field[size / 2][size / 2 + 1]->Liven();
	}

	Cell *getCell(int i, int j) {
		return _field[i][j];
	}

	bool isValidCoordinate(int i, int j) {
		return (i >= 0 && j >= 0 && i < size && j < size);
	}

	void setField(Cell *field[size][size]) {
		for (int i = 0; i < size; i++)
			for (int j = 0; j < size; j++)
				_field[i][j] = field[i][j];
	}

	int Configure() {
		int i = 0, j = 0;
			cin >> i;
			if (i == 999)
				return 1;
			cin >> j;
			if (j == 999)
				return 1;
			if (isValidCoordinate(i, j))
				_field[i][j]->Liven();
	}

	void Generation() {
		int neighbor_count = 0;

		for (int i = 1; i < size - 1; i++) {
			for (int j = 1; j < size - 1; j++) {
				if (isValidCoordinate(i - 1, j - 1))
					if (_field[i - 1][j - 1]->isAlive()) 	neighbor_count++;
				if (isValidCoordinate(i - 1, j + 1))
					if (_field[i - 1][j + 1]->isAlive())	neighbor_count++;
				if (isValidCoordinate(i + 1, j - 1))
					if (_field[i + 1][j - 1]->isAlive()) 	neighbor_count++;
				if (isValidCoordinate(i + 1, j + 1))
					if (_field[i + 1][j + 1]->isAlive()) 	neighbor_count++;
				if (isValidCoordinate(i - 1, j))
					if (_field[i - 1][j]->isAlive()) 		neighbor_count++;
				if (isValidCoordinate(i, j - 1))
					if (_field[i][j - 1]->isAlive())		neighbor_count++;
				if (isValidCoordinate(i, j + 1))
					if (_field[i][j + 1]->isAlive())		neighbor_count++;
				if (isValidCoordinate(i + 1, j))
					if (_field[i + 1][j]->isAlive()) 		neighbor_count++;

				if (2 > neighbor_count || neighbor_count > 3)
					_field[i][j]->Kill();
				if (neighbor_count == 3)
					_field[i][j]->Liven();
				neighbor_count = 0;
				//cout << _field[i][j]->isAlive() << "|" << neighbor_count << "|";
			}
			//cout << endl;
		}
	}
};

class Print
{
public:
	void show(Field *curField) {
			Cell *curCell;
			for (int i = 0; i < size; i++) {
				for (int j = 0; j < size; j++) {
					curCell = curField->getCell(i, j);
					if (curCell->isAlive())
						cout << "*";
					else 
						cout << ".";
				}
				cout << endl;
			}
		}
	void show(Cell *_field[size][size]) {
		Field *tmpField = new Field();
		tmpField->setField(_field);
		show(tmpField);
	}
};