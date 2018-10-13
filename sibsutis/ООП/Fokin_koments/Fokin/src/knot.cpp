#include "knot.h"
#include "match.h"

#include <iostream>

using namespace std;


Knot::Knot() {

	// Заполняем кучку спичками
	for (int i = 0; i < 100; i++) {
		heap[i] = new Match();
	}
	// Задаем, что теперь у нас есть 100 спичек
	matchCount = 100;
}

//Получаем кол-во спичек
int Knot::getMatchCount() {
	return matchCount;
}

// Так, вот тут интересно. Игра сделана так, что первый игрок берет спички
// с левого края, в второй игрок с правого. В итоге, они постепенно движутся к центру
void Knot::hideMatch(int range, bool firstPlayerStep) {

	if (firstPlayerStep) { 				//Если ходит первый игрок
		int lastNoHide = 0;
		for (int i = 0; i < 100; i++) { // то мы слева на право и ищем последнюю "включенную" спичку
			if (heap[i]->getVisible()) {// если мы ее нашли
				lastNoHide = i;			// запоминаем ее номер
				break;
			}
		}

		// Выключаем спички в диапозоне который задал игрок, от последней включенной спички
		// слева на право есесьно
		for (int i = lastNoHide; i < (range + lastNoHide); i++) {
			heap[i]->setVisible(false);
		}
	}
	else {								// Если ходит второй игрок
		int lastNoHide = 0;				// то мы делаем обсалютно тоже самое
		for (int i = 99; i > 0; --i) {	// только справа на лево
			if (heap[i]->getVisible()) {
				lastNoHide = i;
				break;
			}
		}

		for (int i = lastNoHide; i > (lastNoHide - range); i--) {
			heap[i]->setVisible(false);
		}
	}
}

// Узнаем число видимых игроку спичек.
// просто бежим по куче и считаем спички которые видно
int Knot::getVisibleMatchCount() {

	int count = 0;
	for (int i = 0; i < matchCount; i++)
		if (heap[i]->getVisible())
			count++;

	return count;
}

// Узнает видно или нет КОНКРЕТНУЮ спичку
bool Knot::getMatchVisible(int i) {

	if (isValidIndex(i)) {
		return heap[i]->getVisible();
	}
	return false;
}

// проверка на корректность индекса
bool Knot::isValidIndex(int index) {

	return (0 < index && index < 100);
}

// если кучка нам больше не нужна, удаляем ее
Knot::~Knot() {

	delete[] heap;
}
