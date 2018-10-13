#ifndef VIEWER_H
#define VIEWER_H

#include "dot.h"
#include "field.h"
#include <iostream>

using namespace std;

class View 
{
public:
	void printField(Field *thisField) {
			int height = thisField->getHeight();
			int width = thisField->getWidth();
			Dot *curDot;

			cout << "  ";
			for(int line = 0; line < width; line++)
				cout << "|" << line << "|";
			cout << endl;

			for (int i = 0; i < height; i++) {
				cout << i << " ";
				for (int j = 0; j < width; j++ ) {
					cout << "|";
					curDot = thisField->getDot(i, j);
					if (curDot->isOpen() && curDot->value() && !curDot->isBomb())
						cout << curDot->value();
					if (curDot->isOpen() && !curDot->value() && !curDot->isBomb())
						cout << " ";
					if (!curDot->isOpen())
						cout << "*";
					if (curDot->isOpen() && curDot->isBomb() && !curDot->isFlag())
						cout << "b";
					if (curDot->isFlag())
						cout << "F";
					cout << "|";
				}
			cout << '\n';
			}
		}
};

#endif
