#include "match.h"

// Это конструктор, в нем мы указываем,
// что все только что созданные спички видны по умолчанию
Match::Match() {
	_visible = true;
}

// Задаем свойство "видимости" спички
void Match::setVisible(bool visible) {
	_visible = visible;
}

// Узнаем видно спичку или нет
bool Match::getVisible() {
	return _visible;
}
