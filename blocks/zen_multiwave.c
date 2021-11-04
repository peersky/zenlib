///*
// * multiwave.c
// *
// *  Created on: 23 ???. 2019 ?.
// *      Author: Tim
// */
//
//#include "multiwave.h"
////
////static Zero_Monitor_StructTypeDef ZeroWatch;
////static Zero_Monitor_StructTypeDef ZeroWatch2;
////static Fader_StructTypeDef FaderOut[WAVENUM_MAX];
////static Fader_StructTypeDef FaderIn[WAVENUM_MAX];
////static FadeInOut_HandleTypeDef FaderInOut[WAVENUM_MAX];
//
//static uint32_t wavenum;
//static float fstart = 18.0f;
//float V1[WAVENUM_MAX];
//float VOld[WAVENUM_MAX];
//
////static int tr_cnt_up[WAVENUM_MAX];
//static Wave_Handle_TypeDef wave[WAVENUM_MAX];
//static float * harm_ptr[WAVENUM_MAX];
//void multiwave_init(Ring_Handle_TypeDef * SDRAM_Ring)
//{
//	for(int i=0; i<WAVENUM_MAX; i++)
//	{
//		Wave_init(&wave[i],0,fstart);
//		harm_ptr[i]=wave[i].ptr;
//	}
//	wavenum++;
//}
//
//static float bank_lpf = 0.01f;
//static float harmc=0.5f;
//static int dir;
//float multiwave_read(uint32_t size)
//{
//	float retval=0;
//	float v1,v2;
//
//	float power = (int)(1U<<wavenum);
//	float trans_l = 0.15f*fstart;
//	float fqmin = (fstart)+trans_l;
//	float fqmax = (fstart*2.0f*power)-trans_l;
//
//	float m2=-1.0f/(trans_l);
//	float m1=-1.0f*m2;
//	float b1=1.0f-m1*(fqmin);
//	float b2=-1.0f*(fqmax)*m2;
//	float balance;
//	for (int i=0; i<wavenum; i++)
//	{
//
//		if(wave[i].f<(fqmin))
//		{
//			harmc=2.0f;
//			balance = m1*wave[i].f+b1;
//
//			if((balance<=0.0f)||(wave[i].f<=MULTIWAVE_MIN_F))
//			{
//				balance=1.0f;
//				wave[i].f=(fstart*harmc*power);
//				wave[i].ptr=harm_ptr[i];
//				v1= Wave_Read_Next_int(i);
//				debug_print("%s","Bottom. New Freq = ");
//				debug_print("%.1f [%]\n",wave[i].f);
//			}
//			else
//			{
//				v1= Wave_Read_Next(i);
//				v2= Wave_Read_Harmonic(i,harmc);
//				//				assert_param(balance<=1.0f);
//				v1 = local_mixer(v2,v1,balance);
//			}
//
//		}
//		else if((wave[i].f>fqmax))
//		{
//			harmc=0.5f;
//			balance = m2*wave[i].f+b2+1.0f;
//			if(balance<0.0f)
//			{
//				balance=1.0f;
//				wave[i].f=(wave[i].f*harmc);
//				wave[i].ptr=harm_ptr[i];
//				v1= Wave_Read_Next_int(i);
//				debug_print("%s","Top. New Freq = ");
//				debug_print("%.1f [%]\n",wave[i].f);
//			}
//			else
//			{
//				v1= Wave_Read_Next(i);
//				v2= Wave_Read_Harmonic(i,harmc);
//				//				assert_param(balance<=1.0f);
//				v1 = local_mixer(v2,v1,balance);
//			}
//		}
//		else
//		{
//			v1= Wave_Read_Next_int(i);
//		}
//
//		v1 = LPF_f32(VOld[i],v1,0.45);
//		VOld[i]=v1;
//		retval+=v1;
//	}
//
//	return retval;
//}
//
//
//void multiwave_init_shphd_frqncies(void)
//{
//	float power;
//	for (int i=0; i<wavenum; i++)
//	{
//		power=(int)1U<<i;
//		wave[i].f=fstart*power;
//	}
//}
//
//void multiwave_update_parameters(uint32_t new_wavenum, float new_f, float new_fshift, float new_fstep)
//{
//	wavenum=new_wavenum;
//	fstart=new_f;
//	Multiwave_Shephard_Move(new_fshift+new_fstep,0); //Multiwave_Harmonical_Move
//
//}
//
//static inline void Multiwave_Shephard_Move(float fshiftdif, float fader_length)
//{
//	float power;
//	wave[0].f = freq_move(wave[0].f,fshiftdif);
////	if(wave[0].f>=2.0f*fstart)
////		wave[0].f=wave[0].f*0.5f*(1.0f-0.15f);
//
//	for(int y=0; y<wavenum; y++)
//	{
//		power = (int)1U<<y;
//		wave[y].f = wave[0].f*power;
//	}
//}
//
//float multiwave_shepherd_read(uint32_t size)
//{
//	float retval=0;
//	float v1,v2;
//	float power = (int)(1U<<(wavenum-1));
//	float trans_l = 0.45f*fstart*power;
//	float fqmin = (fstart)+trans_l;
//	float fqmax = ((fstart*power*2.0f)-trans_l);
//
//	float m2=-1.0f/(trans_l);
//	float m1=-1.0f*m2;
//	float b1=1.0f-m1*(fqmin);
//	float b2=-1.0f*(fqmax)*m2;
//
//
//	for (int i=0; i<wavenum; i++)
//	{
//		power=(int)(1U<<i);
//		float balance;
//
//		if((wave[i].f>fqmax)&&(i==(wavenum-1)))
//		{
//			balance = m2*wave[i].f+b2+1.0f;
//			if(balance<0.0f)
//			{
//
//				for(int y=wavenum-1; y>0; y--)
//				{
//					wave[y]=wave[y-1];
//				}
//				balance=1.0f;
//				wave[0].f=(wave[0].f*0.5f);
//				wave[0].ptr=harm_ptr[0];
//				v1= Wave_Read_Next_int(0);
//			}
//			else
//			{
//				v1= Wave_Read_Next_int(i);
//				v2= Wave_Read_Harmonic(0,0.5f);
//				v1 = local_mixer(v2,v1,balance);
//			}
//		}
//		else if((wave[i].f<fqmin)&&(i==0))
//		{
//			balance = m1*wave[i].f+b1;
//			if(balance<0.0f)
//			{
//
//				balance=1.0f;
//				wave[wavenum-1].f=(wave[0].f*2.0f*(int)(1U<<31));
//				wave[wavenum-1].ptr=harm_ptr[0];
//				for(int y=0; y<wavenum-1; y++)
//				{
//					wave[y]=wave[y+1];
//				}
//				v1= Wave_Read_Next_int(wavenum-1);
//			}
//			else
//			{
//				v1= Wave_Read_Next_int(i);
//				v2= Wave_Read_Harmonic(wavenum-1,2.0f);
//				v1 = local_mixer(v2,v1,balance);
//			}
//		}
//		else
//		{
//			v1= Wave_Read_Next_int(i);
//		}
//
//		v1 = LPF_f32(VOld[i],v1,0.45f*bank_lpf);
//		bank_lpf = bank_lpf < 1.0f? bank_lpf+0.01f : 1.0f;
//		VOld[i]=v1;
//		retval+=v1;
//	}
//
//	return retval;
//}
//
//
//
//
//
//static inline float Wave_Read_Next(int i)
//{
//	float intpart_f=0;
//	float fltpart;
//	int intpart;
//
//	fltpart = localmodf(wave[i].f,&intpart);
//	float new_balance = fltpart + wave[i].balance;
//	if(new_balance>1.0f)
//	{
//		intpart++;
//		new_balance-=1.0f;
//	}
//	wave[i].balance=new_balance;
//	wave[i].ptr = LocalRing_Point(wave[i].hRing,intpart,wave[i].ptr);
//	return get_wave_val(i);
//}
//
//static inline float Wave_Read_Harmonic(int i, float multiplier)
//{
//	//	float intpart_f=0;
//	//	float fltpart;
//	//	int intpart;
//
//	int multi_f = (int)((float)(multiplier*wave[i].f));
//	//	float multi_b = multiplier*wave[i].balance;
//	//	fltpart = localmodf(multi_f,&intpart);
//	//	float new_balance = fltpart + multi_b;
//	//
//	//	if(new_balance>1.0f)
//	//	{
//	//		intpart++;
//	//		new_balance-=1.0f;
//	//	}
//
//	harm_ptr[i] = LocalRing_Point(wave[i].hRing,multi_f,harm_ptr[i]);
//	float v1 = harm_ptr[i][0];
//	//	float v2 = LocalRing_Point(wave[i].hRing,1,harm_ptr[i])[0];
//
//	//	float retval=local_mixer(v1,v2,wave[i].balance);
//	return v1;
//}
//
//
//static inline float Wave_Read_Next_int(int i)
//{
//	int intpart = (int)wave[i].f;
//	wave[i].ptr = LocalRing_Point(wave[i].hRing,intpart,wave[i].ptr);
//	return get_wave_val(i);
//}
//
////much faster than calling modf from math.h. No condom sex - feelings can overflow.
//static inline float localmodf(float input, int * intpart)
//{
//	*intpart = (int)input;
//	return (input - *intpart);
//}
//
//static inline void wave_restart(int i)
//{
//	wave[i].f = fstart;
//}
//
//static inline float get_wave_val(int i)
//{
//	float v1 = wave[i].ptr[0];
//	float v2 = LocalRing_Point(wave[i].hRing,1,wave[i].ptr)[0];
//	float retval=local_mixer(v1,v2,wave[i].balance);
//	return retval;
//}
//
//static inline float local_mixer(float Data2, float Data1, float balance)
//{
//	float RetVal;
//	//	assert_param(balance>0.0f);
//
//	//	balance = balance>1.0 ? 1.0f : balance;
//
//	Data1=Data1*balance;
//	Data2=Data2*(1.0f-balance);
//	RetVal=Data1+Data2;
//	//	debug_print("D1:%d D2:%d Rv:%d B:%d\n",Data1, Data2, RetVal, balance);
//	return RetVal;
//}
//
//void multiwave_restart(void)
//{
//	for(int i=0; i<wavenum; i++)
//	{
//		wave_restart(i);
//	}
//}
//
//void multiwave_next_bank(void)
//{
//	for(int i=0; i<wavenum; i++)
//	{
//		wave[i].hRing = wave_get_next_table_pointer(wave[i].hRing);
//	}
//	bank_lpf = 0.01f;
//
//}
//
//void mutiwave_set_num(uint32_t num)
//{
//	wavenum = num > WAVENUM_MAX ? WAVENUM_MAX : num;
//}
//
//void multiwave_set_f0(float f)
//{
//	fstart=f;
//}
//
//
//static inline void Multiwave_Harmonical_Move(float fshiftdif, float fader_length)
//{
//	float power;
//	for(int y=0; y<wavenum; y++)
//	{
//		power = (int)1U<<y;
//		float step = fshiftdif*power;
//		wave[y].f = freq_move(wave[y].f,step);
//	}
//}
//
//inline void multiwave_set_num(uint32_t arg)
//{
//	wavenum=arg;
//
//}
//
//static inline float * LocalRing_Point(Ring_Handle_TypeDef * pRing, int offset, float * Ptr)
//{
//	float * retval;
//	uint32_t index;
//
//	//	if(pRing->hParent!=NULL)
//	//	{
//	//		index = Ptr - pRing->hParent->pBuf;
//	//		index = index % (pRing->hParent->pBuf - pRing->pBuf);
//	//		Ptr = &pRing->pBuf[index];
//	//	}
//	if(pRing->hParent!=NULL)
//	{
//		if(Ptr<pRing->pBuf)
//		{
//			uint32_t temp_index;
//			temp_index = Ptr - pRing->hParent->pBuf;
//			temp_index = pRing->hParent->size+temp_index;
//			Ptr = &pRing->hParent->pBuf[temp_index];
//		}
//	}
//
//	index = (Ptr - pRing->pBuf); //head index
//	index = (index + (pRing->size + offset)) % pRing->size;
//	//	assert_param(index<=pRing->size);
//	retval = &pRing->pBuf[index];
//
//	if(pRing->hParent!=NULL)
//	{
//		retval = LocalRing_Point(pRing->hParent,index,pRing->pBuf);
//	}
//
//
//	return retval;
//}
//
//static inline float freq_move(float freq, float step)
//{
//	float retval = freq+step;
//	retval = retval > MULTIWAVE_MIN_F ?
//			retval < MULTIWAVE_MAX_F ? retval : MULTIWAVE_MAX_F
//					: MULTIWAVE_MIN_F;
//	return retval;
//}
//
//static inline float localfabs(float arg)
//{
//	float retval = arg>0.0f ? arg : -1.0f*arg;
//	return retval;
//
//}
