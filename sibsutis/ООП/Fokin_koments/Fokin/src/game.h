#ifndef GAME_H
#define GAME_h

class Knot;

// Класс, который реализует собсна сам цикл игры
class Game
{
private:
	// значение, которое ввел игрок, т.е. кол-во спичек которое нужно убрать
	int _range;
	// флаг, который показывает какой игрок сейчас должен ходить
	bool firstPlayerStep;

	// метод, который спрашивает у пользователя сколько спичек нужно убрать
	void getRange(Knot *);
	// метод, который проверяет корректное ли значение ввел пользователь
	bool rangeIsValid(Knot *);

public:
	// конструктор, в котором говорим что первым будет ходить игрок1
	Game();
	// метод, в котором и происходит сама игра
	void StartGame();
};
#endif