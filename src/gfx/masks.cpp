#include "masks.h"

bool checkbit(int& number, int bit_number)
{
	if((bit_number > 31)||(bit_number < 0))
		return 0;

	if((number&(1<<bit_number)) != 0)
		return 1;

	else
		return 0;
}

void dosetbit(int& number, int bit_number)
{
	if((bit_number > 31)||(bit_number < 0))
		return;

	number = (number|(1<<bit_number));
}

void unsetbit(int& number, int bit_number)
{
	if((bit_number > 31)||(bit_number < 0))
		return;

	int temp = 0;

	if(bit_number != 0)
		temp = ( (0xffffffff<<(bit_number+1)) | (0xffffffff>>(32-bit_number)) );
	else
		temp = (0xffffffff<<(bit_number+1));

	number = number&temp;
}

void flipbit(int& number, int bit_number)
{
	if((bit_number > 31)||(bit_number < 0))
		return;

	if(checkbit(number, bit_number) == 1)
	{
		unsetbit(number, bit_number);
	}
	else
	{
		dosetbit(number, bit_number);
	}
}

float Delta(float a, float b)
{
	float ans = a-b;
	if(ans < 0)
		return (-1.0 * ans);
	else
		return ans;
}	
