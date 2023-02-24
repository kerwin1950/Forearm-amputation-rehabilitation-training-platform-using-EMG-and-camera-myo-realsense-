#pragma once
#include"Common.h"
#define EPS 1e-8
#define SAMP 50
class TDextraction
{
public:
	float Mav(float x[SAMP]);
	float Zc(float x[SAMP]);
	float Ssc(float x[SAMP]);
	float Wl(float x[SAMP]);
	int sgn(float d);
	int sgnw(float d, float w);
};
void SVMtrain();
void SVMtest();
int SVM_realtimetest(float x[8][50]);
