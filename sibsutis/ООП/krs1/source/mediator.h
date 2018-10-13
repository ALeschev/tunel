#ifndef MEDIATOR_H
#define MEDIATOR_H

#include "field.h"
#include "viewer.h"

class Mediator
{
public:
	Mediator(Field *, View *);
	void ViewField();
private:
	Field *_field;
	View  *_viewer;
};

#endif
