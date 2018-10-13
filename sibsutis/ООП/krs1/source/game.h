#ifndef GAME_H
#define GAME_H

class Game
{
	int  _x, _y;
	int  _fieldHeight;
	int  _fieldWidth;
	int  _bombNumber;
	bool _setFlag;

public:
	void StartGame();
	
private:
	void setFlag(bool );
	bool flagIsSet();
	void getBombNumber();
	void getCoordinates();
};

#endif
