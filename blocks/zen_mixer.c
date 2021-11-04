/*
 * mixer.c
 *
 *  Created on: 13 ????. 2018 ?.
 *      Author: Tim
 */

#include "zen_mixer.h"



//100 balance = fully data1, balance 0 -> fully data2
signed int Mixer_sign_long(signed long long  Data2, signed long long Data1, signed int balance)
{
	signed int RetVal;
	//	assert_param(K<100);
	if (balance>100)
	{
		balance=100;
	}
	Data1=Data1*((long long)balance)/100;
	Data2=Data2*(100-(long long)balance)/100;
	RetVal=Data1+Data2;
	RetVal = RetVal & 0xFFFFFF00;
	//	debug_print("D1:%d D2:%d Rv:%d B:%d\n",Data1, Data2, RetVal, balance);
	return (signed int)RetVal;
}


//100 balance = fully data1, balance 0 -> fully data2
inline float Mixer_f(float Data2, float Data1, float balance)
{
	float RetVal;
//	assert_param(balance>0.0f);

	balance = balance>1.0 ? 1.0f : balance;

	Data1=Data1*balance;
	Data2=Data2*(1.0f-balance);
	RetVal=Data1+Data2;
	//	debug_print("D1:%d D2:%d Rv:%d B:%d\n",Data1, Data2, RetVal, balance);
	return RetVal;
}
Data1 * Balance + Data2 - Data2 * Balance
Data2 + ( Data1 - Data 2 ) * Balance
