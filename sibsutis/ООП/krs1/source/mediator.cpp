#include "field.h"
#include "mediator.h"
#include "field.h"
#include "viewer.h"

Mediator::Mediator(Field *newField, View *Viewer) {
	_field = newField;
	_viewer = Viewer;
}

void Mediator::ViewField() {
	_viewer->printField(_field);
}
