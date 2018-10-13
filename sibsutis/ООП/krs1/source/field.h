#ifndef FIELD_H
#define FIELD_H

class Dot;

class Field
{
	Dot  *field[8][8];
	int  _height;
	int  _width;
	int  _bombNumber;
	bool _gameIsActive;
	bool _win;

public:
	Field(int );
	Field(const Field &);

	Dot* getDot(int i, int j);
	int  getHeight();
	int  getWidth();
	int  getBombNumber();
	void Step(int , int , bool );
	bool gameIsActive();
	bool Winner();

	~Field();

private:
	void openAll();
	void openDot(int , int );
	bool isWin();
	bool isValidCoordinate(int , int );
};

#endif