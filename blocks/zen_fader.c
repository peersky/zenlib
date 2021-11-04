/*
 * fader.c
 *
 *  Created on: 13 ????. 2018 ?.
 *      Author: Tim
 */

#include "zen_fader.h"

void FadeInOut_ms_Init(FadeInOut_HandleTypeDef * hFadeInOut, float transition_length_ms, uint32_t Fs)
{
	FadeInOut_Init(hFadeInOut,(int)transition_length_ms*Fs,0);
//	hFadeInOut->step_num = transition_length_ms*(float)Fs;
//	hFadeInOut->min_level=0.000001f;
//	hFadeInOut->level = 1.0f;
//	hFadeInOut->step_size = 2*(hFadeInOut->level-hFadeInOut->min_level)/(transition_length_ms*Fs);
//
//
//	hFadeInOut->counter=hFadeInOut->step_num;
//	hFadeInOut->activated=1;
//	hFadeInOut->Transition_Fs = (float)Fs;
//	hFadeInOut->transition_length_ms = transition_length_ms;

//	hFadeInOut->step_len_samples = transition_length_ms/hFadeInOut->step_num;

}

float FadeInOut_ms(float InVal, FadeInOut_ms_HandleTypeDef * hFadeInOut)
{
//	float RetVal;
//
//	//FadeIn out
//	if(hFadeInOut->activated)
//	{
//		//Fade In if <50 or Fade Out if > 50
//		if(hFadeInOut->counter<50)
//		{
//			RetVal = Fader(InVal,(float)2*hFadeInOut->counter);
//		}
//		else
//		{
//			RetVal = Fader(InVal,(float)100-2*(hFadeInOut->counter-50));
//		}
//		hFadeInOut->subcounter++;
//		if ((hFadeInOut->subcounter*hFadeInOut->Transition_Fs)>=(hFadeInOut->transition_length_ms)/100.0f)
//		{
//			hFadeInOut->subcounter=0;
//			hFadeInOut->counter--;
//			if (hFadeInOut->counter==0)
//			{
//				hFadeInOut->activated=0;
//			}
//		}
//
//	}
//	else
//	{
//		RetVal = InVal;
//	}
//
//	return RetVal;
	return 0;

}

float FadeInOut_ms_Stereo(float InVal, FadeInOut_ms_HandleTypeDef * hFadeInOut, uint32_t channel)
{
	float RetVal;
	//FadeIn out
	if(hFadeInOut->activated)
	{
		if(channel==0)
		{
			hFadeInOut->counter--;
			if((hFadeInOut->counter*2)>=(hFadeInOut->step_num))
			{
				hFadeInOut->level=hFadeInOut->level - hFadeInOut->step_size;
			}
			else
			{
				hFadeInOut->level+=hFadeInOut->step_size;
				if(hFadeInOut->level>=1)
				{
					hFadeInOut->level=1.0f;
				}
			}
			if (hFadeInOut->counter==0) hFadeInOut->activated=0;
		}


		if(hFadeInOut->level>0.0001f)
		{
			RetVal = Fader_f(InVal,hFadeInOut->level);

		}
		else
		{
			RetVal=0;
		}
	}
	else
	{
		RetVal = InVal;
	}

	return RetVal;

}

void FadeIn_Init(Fader_StructTypeDef * hFadeIn, uint32_t transition_length)
{
	Fader_Init(hFadeIn,transition_length,0);
}

void FadeOut_Init(Fader_StructTypeDef * hFadeIn, uint32_t transition_length)
{
	Fader_Init(hFadeIn,transition_length,120);
}

void FadeInOut_Init(FadeInOut_HandleTypeDef * hFadeInOut, uint32_t transition_length, uint32_t silence_length)
{

	Fader_Init(&hFadeInOut->FadeIn,transition_length/2,0);
	Fader_Init(&hFadeInOut->FadeOut,transition_length/2,120);
	hFadeInOut->Silence_length = silence_length + transition_length % 2;
	hFadeInOut->activated=1;

}

void Fader_Init(Fader_StructTypeDef * hFadeIn, uint32_t transition_length, uint32_t starting_level)
{

	assert_param(transition_length);

	hFadeIn->transition_length = transition_length;

	if(hFadeIn->transition_length>120)
	{
		hFadeIn->leftover =  hFadeIn->transition_length % 120;
		hFadeIn->transition_length = hFadeIn->transition_length - hFadeIn->leftover;
	}
	else
	{
		hFadeIn->leftover = 0;
	}

	hFadeIn->steps =  hFadeIn->transition_length>=120 ? 120 : hFadeIn->transition_length;
	hFadeIn->db_in_a_step_1 = 120/hFadeIn->steps;
	hFadeIn->db_in_a_step_2 = 120%hFadeIn->steps;

	hFadeIn->samples_per_step = hFadeIn->transition_length / hFadeIn->steps;

	hFadeIn->counter=0;
	hFadeIn->activated=1;
	hFadeIn->level=starting_level;
}

float FadeInOut(float InVal, FadeInOut_HandleTypeDef * hFadeInOut)
{
	if(hFadeInOut->activated)
	{
		if(hFadeInOut->FadeIn.activated)
		{
			InVal = FadeIn(InVal,&hFadeInOut->FadeIn);
			hFadeInOut->level_cpy = hFadeInOut->FadeIn.level;
		}
		else if(hFadeInOut->Silence_length)
		{
			hFadeInOut->Silence_length--;
			InVal=0.0f;
			hFadeInOut->level_cpy = 121;
		}
		else if(hFadeInOut->FadeOut.activated)
		{
			InVal = FadeOut(InVal,&hFadeInOut->FadeOut);
			hFadeInOut->level_cpy = hFadeInOut->FadeOut.level;
		}
		else
		{
			hFadeInOut->activated=0;
		}
	}
	return InVal;

}
void FadeInOut_Stereo(float * InVal, FadeInOut_HandleTypeDef * hFadeInOut)
{

	if(hFadeInOut->activated)
	{
		if(hFadeInOut->FadeIn.activated)
		{
			FadeIn_Stereo(InVal,&hFadeInOut->FadeIn);
			hFadeInOut->level_cpy = hFadeInOut->FadeIn.level;
		}
		else if(hFadeInOut->Silence_length)
		{
			hFadeInOut->Silence_length--;
			InVal[0]=0.0f;
			InVal[1]=0.0f;
			hFadeInOut->level_cpy = 121;
		}
		else if(hFadeInOut->FadeOut.activated)
		{
			FadeOut_Stereo(InVal,&hFadeInOut->FadeOut);
			hFadeInOut->level_cpy = hFadeInOut->FadeOut.level;
		}
		else
		{
			hFadeInOut->activated=0;
		}
	}
//	if(hFadeInOut->activated)
//	{
//		if(hFadeInOut->counter%hFadeInOut->samples_per_step == 0)
//		{
//			if(hFadeInOut->counter<hFadeInOut->Fade_in_length)
//			{
//				hFadeInOut->level += hFadeInOut->db_in_a_step_1;
//				if(hFadeInOut->db_in_a_step_2>0)
//				{
//					hFadeInOut->level++;
//					hFadeInOut->db_in_a_step_2--;
//				}
//
//				hFadeInOut->level = hFadeInOut->level>120 ? 120 : hFadeInOut->level;
//			}
//			else if (hFadeInOut->counter<(hFadeInOut->Fade_in_length+hFadeInOut->Silence_time))
//			{
//				hFadeInOut->level = 120;
//			}
//			else
//			{
//				hFadeInOut->level -= hFadeInOut->db_in_a_step_1;
//				if(hFadeInOut->db_in_a_step_3>0)
//				{
//					hFadeInOut->level--;
//					hFadeInOut->db_in_a_step_3--;
//				}
//
//				hFadeInOut->level = hFadeInOut->level>120 ? 0 : hFadeInOut->level;
//			}
//		}
//
//		hFadeInOut->counter++;
//		if(hFadeInOut->counter >= hFadeInOut->transition_length)
//		{
//			hFadeInOut->counter=0;
//			hFadeInOut->activated=0;
//
//		}
//		else
//		{
//			InVal[0] = Fader_log(InVal[0],hFadeInOut->level);
//			InVal[1] = Fader_log(InVal[1],hFadeInOut->level);
//		}
//	}

}
//float FadeInOut_Stereo(float InVal, FadeInOut_HandleTypeDef * hFadeInOut, uint8_t channel)
//{
//	float RetVal;
//	float level;
//	if(hFadeInOut->transition_length<100) hFadeInOut->transition_length=100;
//	//FadeIn out
//	if(hFadeInOut->activated)
//	{
//		float f_counter = (float)(hFadeInOut->counter);
//		f_counter = f_counter / 100.0f;
//		//Fade In if >50 or Fade Out if < 50
//		if(hFadeInOut->counter<50)
//		{
//			level = (1.0f-f_counter*2.0f);
//			RetVal = Fader_f(InVal,level);
//		}
//		else
//		{
//			level = 2.0f*f_counter-1.0f;
//			RetVal = Fader_f(InVal,level);
//		}
//		if(channel==0)
//		{
//			hFadeInOut->subcounter++;
//			if (hFadeInOut->subcounter>=(hFadeInOut->transition_length/100))
//			{
//				hFadeInOut->subcounter=0;
//				hFadeInOut->counter--;
//				if (hFadeInOut->counter==0)
//				{
//					hFadeInOut->activated=0;
//				}
//			}
//		}
//	}
//	else
//	{
//		RetVal = InVal;
//	}
//
//	return RetVal;
//
//}

float FadeIn(float InVal, Fader_StructTypeDef * hFadeIn)
{
	if(hFadeIn->activated)
	{
		hFadeIn->counter++;
		if(hFadeIn->counter >= hFadeIn->transition_length)
		{
			hFadeIn->counter=0;
			hFadeIn->activated=0;
		}

		uint32_t samples_this_step = hFadeIn->leftover ? hFadeIn->samples_per_step+1 : hFadeIn->samples_per_step;
		hFadeIn->leftover = hFadeIn->leftover ? hFadeIn->leftover-1 : 0;
		if(hFadeIn->counter%samples_this_step == 0)
		{
			hFadeIn->level += hFadeIn->db_in_a_step_1;
			if(hFadeIn->db_in_a_step_2>0)
			{
				hFadeIn->level++;
				hFadeIn->db_in_a_step_2--;
			}
			hFadeIn->level = hFadeIn->level>120 ? 120 : hFadeIn->level;
		}



	}
		return Fader_log(InVal,hFadeIn->level);

}

void FadeIn_Stereo(float * InVal, Fader_StructTypeDef * hFadeIn)
{

	if(hFadeIn->activated)
	{
		hFadeIn->counter++;
		if(hFadeIn->counter >= hFadeIn->transition_length)
		{
			hFadeIn->counter=0;
			hFadeIn->activated=0;
		}

		uint32_t samples_this_step = hFadeIn->leftover ? hFadeIn->samples_per_step+1 : hFadeIn->samples_per_step;
		hFadeIn->leftover = hFadeIn->leftover ? hFadeIn->leftover-1 : 0;
		if(hFadeIn->counter%samples_this_step == 0)
		{
			hFadeIn->level += hFadeIn->db_in_a_step_1;
			if(hFadeIn->db_in_a_step_2>0)
			{
				hFadeIn->level++;
				hFadeIn->db_in_a_step_2--;
			}
			hFadeIn->level = hFadeIn->level>120 ? 120 : hFadeIn->level;
		}



	}
		InVal[0] = Fader_log(InVal[0],hFadeIn->level);
		InVal[1] = Fader_log(InVal[1],hFadeIn->level);

}

float FadeOut(float InVal, Fader_StructTypeDef * hFadeOut)
{
	if(hFadeOut->activated)
	{
		hFadeOut->counter++;
		if(hFadeOut->counter >= hFadeOut->transition_length)
		{
			hFadeOut->counter=0;
			hFadeOut->activated=0;
		}
//		else
		{
			uint32_t samples_this_step = hFadeOut->leftover ? hFadeOut->samples_per_step+1 : hFadeOut->samples_per_step;
			hFadeOut->leftover = hFadeOut->leftover ? hFadeOut->leftover-1 : 0;
			if(hFadeOut->counter%samples_this_step == 0)
			{
				hFadeOut->level -= hFadeOut->db_in_a_step_1;
				if(hFadeOut->db_in_a_step_2>0)
				{
					hFadeOut->level--;
					hFadeOut->db_in_a_step_2--;
				}
				hFadeOut->level = hFadeOut->level>120 ? 0 : hFadeOut->level;
			}
		}
	}
			return Fader_log(InVal,hFadeOut->level);
}

void FadeOut_Stereo(float * InVal, Fader_StructTypeDef * hFadeOut)
{
	if(hFadeOut->activated)
	{
		hFadeOut->counter++;
		if(hFadeOut->counter >= hFadeOut->transition_length)
		{
			hFadeOut->counter=0;
			hFadeOut->activated=0;
		}
//		else
		{
			uint32_t samples_this_step = hFadeOut->leftover ? hFadeOut->samples_per_step+1 : hFadeOut->samples_per_step;
			hFadeOut->leftover = hFadeOut->leftover ? hFadeOut->leftover-1 : 0;
			if(hFadeOut->counter%samples_this_step == 0)
			{
				hFadeOut->level -= hFadeOut->db_in_a_step_1;
				if(hFadeOut->db_in_a_step_2>0)
				{
					hFadeOut->level--;
					hFadeOut->db_in_a_step_2--;
				}
				hFadeOut->level = hFadeOut->level>120 ? 0 : hFadeOut->level;
			}
		}
	}
			InVal[0] = Fader_log(InVal[0],hFadeOut->level);
			InVal[1] = Fader_log(InVal[1],hFadeOut->level);
}



//float FadeOut_Stereo_f_ms(float InVal, FadeInOut_f_ms_HandleTypeDef * hFadeInOut, uint8_t channel)
//{
//	float RetVal;
//	if(hFadeInOut->transition_length<100) hFadeInOut->transition_length=100;
//
//	//FadeIn out
//	if(hFadeInOut->activated)
//	{
//		if(channel==0)
//		{
//			hFadeInOut->subcounter++;
//			if (hFadeInOut->subcounter>=(hFadeInOut->transition_length/100))
//			{
//				hFadeInOut->subcounter=0;
//				hFadeInOut->counter--;
//				if (hFadeInOut->counter==0)
//				{
//					hFadeInOut->activated=0;
//				}
//			}
//		}
//		RetVal = Fader(InVal,100-hFadeInOut->counter);
//	}
//	else
//	{
//		RetVal = InVal;
//	}
//
//	return RetVal;
//
//}

float Fader_f(float Sample, float Level)
{

//	assert_param(Level<=1000.0f);
	{
		float retval = (Sample * Level);
		return retval;
	}
	//	printf("Sample: %d, Divider: %d, Result: %d \n", Sample,Divider,retval);

}

float Fader(float Sample, float Divider)
{

	assert_param(Divider<=100.0f);
	{
		float retval = (Sample * Divider / 100.0f);
		return retval;
	}
	//	printf("Sample: %d, Divider: %d, Result: %d \n", Sample,Divider,retval);

}

float Fader_log(float Sample, uint32_t db_level)
{

	db_level = db_level>=121 ? 121 : db_level;
//		assert_param(db_level<=121);
	{
		static float old_gain,old_val;
		float retval;
		float gain = gain_array[db_level];
		//bad idea because on short fades it will be too slow to switch sound completley off.
//		gain = LPF_f32(old_gain,gain,0.001f);
		retval = (Sample * gain);
//		retval = LPF_f32(old_val,retval,0.1f);

		old_gain = gain;
		old_val = retval;

		return retval;
	}

}


float Fader_log_lpf(float Sample, uint32_t db_level)
{

	db_level = db_level>=121 ? 121 : db_level;
//		assert_param(db_level<=121);
	{
		static float old_gain,old_val;
		float retval;
		float gain = gain_array[db_level];
		//bad idea because on short fades it will be too slow to switch sound completley off.
		gain = LPF_f32(old_gain,gain,0.00001f);
		retval = (Sample * gain);
//		retval = LPF_f32(old_val,retval,0.1f);

		old_gain = gain;
		old_val = retval;

		return retval;
	}

}
