#ifndef _NAVSCREENOCCUPIED_H_

#define _NAVSCREENOCCUPIED_H_



//#include "masks.h"



class navscreenoccupied

{

private:



	//	bits are down. sectors across. starting at top left of navscreen area

	int sector0;

	int sector1;

	int sector2;

	int sector3;

	int sector4;

	int sector5;

	int sector6;

	int sector7;

	int sector8;

	int sector9;



	float screenskipby4[4];

	bool markreturned;



public:

	navscreenoccupied();

	navscreenoccupied(float x_small, float x_large, float y_small, float y_large, bool _markreturned);



	void reset();

	float findfreesector(float x, float y);

	float findfreefloat(int& sector, float& y);

};



#endif