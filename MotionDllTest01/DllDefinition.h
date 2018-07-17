#pragma once
#define MotionTest __declspec (dllexport)

struct SendMDA
{
	unsigned int mcw;
	float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s;
	unsigned long dummy1, dummy2;
};


//extern "C" MotionTest void MotionBaseInit(char * h);
//extern "C" MotionTest bool IsMotionBaseInitialized();
extern "C" MotionTest void UpdateMotionBase();
extern "C" MotionTest void SetSmoothing(bool w);
extern "C" MotionTest int MotionStart(char * host);
extern "C" MotionTest void ShutDownMotionBase();
extern "C" MotionTest void SetRoll(float f);
extern "C" MotionTest void SetPitch(float f);
extern "C" MotionTest void SetYaw(float f);
extern "C" MotionTest void SetHeave(float f);
extern "C" MotionTest void SetSurge(float f);
extern "C" MotionTest void SetSway(float f);
extern "C" MotionTest void SetMDA(SendMDA mda_rev);

