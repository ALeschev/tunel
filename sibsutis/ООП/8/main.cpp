#include <iostream>

using namespace std;

class Motor
{
public:
	bool isStarted() {
		return _isStarted;
	}
	virtual void startMotor();
};

class Fuel
{
public:
	bool haveFuel() {
		if (_fuel > 0)
			return true;
		else false;
	}
};

class Wheel
{
public:
	virtual bool isGood();
};

class Car : public Motor, Fuel, Wheel
{
	virtual void startMotor() {
		rotateKey();
		_isStarted = true;
	}

	virtual bool isGood() {
		int readyCount = 0;
		for (int i = 0; i < 4; i++)
			if (wheelReady(i)) 
				readyCount++;
		if (readyCount == 4)
			return true;
		else 
			return false;
	}
};

class Bike : public Wheel
{
	virtual bool isGood() {
		int readyCount = 0;
		for (int i = 0; i < 2; i++)
			if (wheelReady(i)) 
				readyCount++;
		if (readyCount == 2)
			return true;
		else 
			return false;
	}
};

class Motorboat : public Motor, Fuel
{
	virtual void startMotor() {
		pull_cord();
		_isStarted = true;
	}
};