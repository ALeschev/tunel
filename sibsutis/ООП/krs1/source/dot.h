#ifndef DOT_H
#define DOT_H

class Dot
{
	int  _i;
	int  _j;
	int  _value;
	bool _isBomb;
	bool _isOpen;
	bool _isFlag;

public:
	Dot(int , int );
	Dot(const Dot &);

	int  i();
	int  j();
	int  value();
	void setBomb();
	void setOpen();
	void setFlag(bool );
	void incValue();
	bool isBomb();
	bool isOpen();
	bool isFlag();

//	~Dot();
};

#endif
