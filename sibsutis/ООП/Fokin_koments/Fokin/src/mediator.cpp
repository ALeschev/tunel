#include "knot.h"
#include "mediator.h"
#include "print.h"

Mediator::Mediator(Knot *newField, Print *Viewer) {
	// связывает поле и класс который будет его выводить на экран
	_field = newField;
	_viewer = Viewer;
}

void Mediator::ViewField() {
	// вызываем метод который выводит поле на экран
	_viewer->show(_field);
}
