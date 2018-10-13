#include "game.h"
#include "match.h"
#include "knot.h"
#include "print.h"
#include "mediator.h"
#include <iostream>

using namespace std;

Game::Game() {
	// задаем, что первым у нас будет ходить игрок1
	firstPlayerStep = true;
}

void Game::StartGame() {

	// создаем кучку
	Knot *field = new Knot(); 
	// создаем штуку, которая будет рисовать нашу кучку на экране
	Print *view = new Print();

	// связываем эти две вещи через "посредника", 
	// т.е. напрямую мы не будет взаимодействовать ни с одним из этих штуковин
	Mediator mediator(field, view);

	// главный цикл игры, игра идет до тех пор пока спичек в кучке больше 1
	while (field->getVisibleMatchCount() > 1) {

		// выводим на экран нашу кучку
		mediator.ViewField();

		// если ходит игрок 1, то печатаем что ходит игрок 1 :D
		// точно так же со сторым игроком
		if (firstPlayerStep)
			cout << "Player1:" << endl;
		else 
			cout << "Player2:" << endl;
			
		cout << "MatchCount " << field->getVisibleMatchCount() << endl;
		// спрашиваем игрока сколько спичек он хочет убрать
		getRange(field);

		// убираем столько спичек, сколько захотел игрок
		field->hideMatch(_range, firstPlayerStep);

		// и собсна передает ход другому игроку
		if (firstPlayerStep)
			firstPlayerStep = false;
		else
			firstPlayerStep = true;
	}
	// когда спичек осталось 1 или меньше, мы проверяем кто же выиграл
	if (field->getVisibleMatchCount() == 1)
		if (firstPlayerStep)
			cout << "Player2 win" << endl;
		else
			cout << "Player1 win" << endl;
	else
		if (firstPlayerStep)
			cout << "Player1 win" << endl;
		else
			cout << "Player2 win" << endl;

	field->~Knot();

}

void Game::getRange(Knot *k) {

	// спрашиваем у игрока сколько спичек он хочет убрать,
	// если он ввел кракозябру, то мы переспрашиваем его.
	// переспрашивать будем до тех пор, пока он не введет нормальное значение
	do {
		cout << "Enter range (1<= && <=10): ";
		cin >> _range;
	}while (!rangeIsValid(k));
}

bool Game::rangeIsValid(Knot *k) {
	// собсна тут мы и проверяем ввел пользователь все правильно, или его нужно переспросить
	if (1 <= _range && _range <= 10)
		return true;

	return false;
}
