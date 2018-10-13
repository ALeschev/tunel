#include "field.h"
#include "game.h"
#include "mediator.h"
#include "viewer.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

void Game::StartGame() {

	_fieldHeight = 8;
	_fieldWidth  = 8;
	getBombNumber();
	
	Field *curField = new Field(_bombNumber);
	View *viewer    = new View();

	Mediator med(curField, viewer);

	med.ViewField();
	while (curField->gameIsActive()) {
	
		getCoordinates();
		system("clear");
		curField->Step(_x, _y, flagIsSet());
		med.ViewField();
	}

	if (curField->Winner())
		cout << "Вы победили! :)" << endl;
	else 
		cout << "Вы проиграли! :(" << endl;

	curField->~Field();
}

bool Game::flagIsSet() {
	return _setFlag;
}

void Game::setFlag(bool flag) {
	_setFlag = flag;
}

void Game::getCoordinates() {

	char action;
	cout << "Введите строку: ";
	cin >> _x;
	cout << "Введите столбец: ";
	cin >> _y;
	cout << "Открыть(O); Флаг(F): ";
	cin >> action; 

	if (action == 'F')
		setFlag(true);
	else {
		setFlag(false);
		action = '\0';
	}
}

void Game::getBombNumber() {

	system("clear");
	
	while(true) {
		cout << "Введите колличество бомб: ";
		cin >> _bombNumber;

		if ((_bombNumber > 0) && (_bombNumber < _fieldHeight * _fieldWidth))
			break;
		
		if (_bombNumber >= _fieldHeight * _fieldWidth)
			cout << "Слишком много бомб. Попробуйте еще раз." << endl;

		if(_bombNumber <= 0)
			cout << "Слишком мало бомб. Попробуйте еще раз." << endl;
	}

}