#ifndef PRINT_H
#define PRINT_H

#include <iostream>
#include "match.h"
#include "knot.h"
#include <stdlib.h>


using namespace std;

// это класс, который выводит наше поле на экран
// благодаря, паттерну медиатор мы может переписать только его
// и у нас уже будет все не в консольке, а все круто и с кнопочками
class Print
{
public:
	void show(Knot *field) {
		// очищаем экран перед тем как вывести поле 
		system("clear");

		// получаем кол-во спичек
		int matchCount = field->getMatchCount();
		for (int i = 0; i < matchCount; i++) {
			// если спичка видно, т.е. она не убрана, то печатаем подобие спички 
			if (field->getMatchVisible(i))
				cout << "|";
			else // если же спичку уже убрали, то печатаем пустое место
				cout << " ";
		}
		cout << endl;
	}
};
#endif