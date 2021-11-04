//
//  sampler.c
//  ZenPlugin - Shared Code
//
//  Created by Tim on 07/05/2021.
//  Copyright © 2021 Teknologic. All rights reserved.
//

#include "zen_sampler.h"


void Delay_stereo_add(Delay_HandleTypeDef * hDelay, float * RawIn, uint8_t channels_number)
{

    float  temp[2];
    float  fAdder[2];
    float  Adder[2];
    __IO int Adder_limited[2];
    __IO float Adder_IO[2];

    Adder_IO[0] = hDelay->hSubRing.Head[0];
    Adder_IO[1] = hDelay->hSubRing.Head[hDelay->hMem.offset];

    temp[0]=(float)Adder_IO[0];
    temp[1]=(float)Adder_IO[1];
    Adder[0]=RawIn[0];
    Adder[1]=RawIn[1];
    if(hDelay->Add_Flag==1)
    {

        Adder[0]=RawIn[0];
        Adder[1]=RawIn[1];
        FadeOut_Stereo(Adder,&hDelay->Add_FadeInOut);
        if(hDelay->Add_FadeInOut.activated==0)
        {
            hDelay->Add_Flag=0;
        }

    }
    else if (hDelay->Add_Flag==2)
    {
        FadeIn_Stereo(Adder,&hDelay->Add_FadeInOut);
        if(hDelay->Add_FadeInOut.activated==0)
        {
            hDelay->Add_Flag=0;
            hDelay->Playback=HOLD;
        }
    }
    else if (hDelay->Add_Flag==3)
    {
        FadeIn_Stereo(Adder,&hDelay->Add_FadeInOut);

        if(hDelay->Add_FadeInOut.activated==0)
        {
            hDelay->Add_Flag=0;
            hDelay->Playback=PLAYBACK;
        }
    }
    else
    {
        Adder[0] = RawIn[0];
        Adder[1] = RawIn[1];
    }

    //tt is to avoid compilator working on addition of two volatiles..
    float tt[2];
    tt[0]=temp[0];
    tt[1]=temp[1];

    Adder[0]+=tt[0];
    Adder[1]+=tt[1];
    fAdder[0]=compress_struct(Adder[0],&Delay_compressor);
    fAdder[1]=compress_struct(Adder[1],&Delay_compressor);
    Adder_limited[0] = (int)fAdder[0];
    Adder_limited[1] = (int)fAdder[1];

    hDelay->hSubRing.Head[0]=Adder_limited[0];
    hDelay->hSubRing.Head[hDelay->hMem.offset]=Adder_limited[1];

}
