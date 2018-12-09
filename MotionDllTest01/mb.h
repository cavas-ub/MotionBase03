#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <windows.h>

#include <thread>	//rd
#include <winsock.h> //rd
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "defines.h"
#include "duration.h"

#include "DllDefinition.h"

#define HAVE_STRUCT_TIMESPEC

//#include "pthread.h"

//rd  error:
	//LNK2019	unresolved external symbol _bind @12 referenced in function "private: void __thiscall MotionBase::SetupSockets(void)" (? SetupSockets@MotionBase@@AAEXXZ)	motion01	E : \UB\motion\motion01\motion01\mb_win32.obj	1
	//add Ws2_32.lib library in project's linker settings  OR:
#pragma comment (lib, "Ws2_32.lib")
//rd




#define UPDATERATE 16667
#define PI 3.14159265358979323846
#define DTOR (double)PI/(double)180.0
#define RTOD (double)180.0/(double)PI
#define to_neut_sm 0.001			// base rate for 60 Hz.
#define mot_sm 0.004				// base rate for 60 Hz.
#define num_ems 10					// Number of encoded machine states

//from linux  rd
//struct timezone {
//	int tz_minuteswest;     /* minutes west of Greenwich */
//	int tz_dsttime;         /* type of DST correction */
//};


struct SendCommand
{
	// Can be used for DOF or LENGTH mode.
	unsigned int mcw;
	float a,b,c,d,e,f; 
	float dummy;
};

struct Response
{
	// Responses are *only* DOF or LENGTH.
	unsigned long fault;
	unsigned long i_info;
	unsigned long status;
	float a,b,c,d,e,f;
	float dummy;
};


int Running;
int sendsocket;
int receivesocket;
Response r;
SendCommand s, s_rev, hold, goal;
SendMDA sm, sm_rev, sm_rev_oryginal;
float da, db, dc, dd, de, df;//the smoothing factors
int State;//machine state

bool UpdateFlag, SmoothFlag;
struct sockaddr_in cliAddr, remoteSendAddr;



void SetSmoothingValues(float a, float b, float c, float d,
	float e, float f) {
	da = a; db = b; dc = c; dd = d; de = e; df = f;
};


char IPAddress[255];
void SetState(int st) { State = st; };
void SetMotionCommand(unsigned int c)
{
	s.mcw = c;
};


void SetMotionCommandMDA(unsigned int c)
{
	sm.mcw = c;
};

enum VibrationFrequency
{
	Low,
	High
};

unsigned int swapbits(unsigned int varin);
float floatSwap(float f);
float FloatSwap(char * value);
void SetHost(char * h);
char * GetHost();
void fb_params();
void MotionBaseInit();
void ReadMotionBase();
void rbo(void);
void ReadWriteMotionBase(void);
void WriteMotionBase(void);
void SetupSockets();

void Smooth();

