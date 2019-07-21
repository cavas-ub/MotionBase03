#include "stdafx.h"

#include "mb.h"

#include <iostream> //cout
#include <cmath>	//abs()


using std::cerr;
using std::cout;
using std::endl;
using std::string;





// Send and receive ports.
#define REMOTE_SERVER_SEND_PORT 991
#define REMOTE_SERVER_RECEIVE_PORT 992

// Global variables.
char DCIP[32] = { "128.127.55.121" };     // Dedicated Card IP.
//int MotionBase::Running;
//int MotionBase::sendsocket;
//int MotionBase::receivesocket;
//bool MotionBase::UpdateFlag, MotionBase::SmoothFlag;
//float MotionBase::da, MotionBase::db, MotionBase::dc, MotionBase::dd, MotionBase::de, MotionBase::df;
//Response MotionBase::r;
//SendCommand MotionBase::s_rev, MotionBase::s, MotionBase::hold, MotionBase::goal;
//SendMDA MotionBase::sm_rev, MotionBase::sm;
//int MotionBase::State;
//struct sockaddr_in MotionBase::cliAddr, MotionBase::remoteSendAddr;

int alt = 0;
FILE *fpg, *fpgg;

// Motion variables.
int EMS[num_ems];				// Encoded Machine States
int feedback_which = 0;			// Length (1), or DOF (0)
int rev_comm = 1;				// 0 = forward mode, 1 = reverse mode

bool dll_loaded = false;

//int motion_mode = 0;			// 0 = DOF, 1 = LENGTH, 2 = MDA - these are motion COMMANDS

int motion_mode = 2;	//MDA mode



//int MDA_FILE_NO = 25856; //101

int MDA_FILE_NO = 26112; //102
//int MDA_FILE_NO = 26368; //103




double update_speed = 60.0;
double udf = ((double)update_speed / 60.0);

std::thread readThread, writeThread, readwriteThread;


void fb_params()
{
	int i;
	//int default_mda_fn = 25856;  // This is file number "101", starting with bit #9, leftward.
	//default_mda_fn = 26112;  // This is file number "102", starting with bit #9, leftward.

	// (feedback_which)  Define 0 for DOF feedback; 1 for LENGTH feedback(default)
	// (motion_mode)     0 = DOF(default), 1 = LENGTH, 2 = MDA - these are motion COMMANDS

	// Encoded Machine States.
	EMS[0] = 0;   // Power_up
	EMS[1] = 1;   // Idle
	EMS[2] = 2;   // Standby
	EMS[3] = 3;   // Engaged
	EMS[4] = 7;   // Parking
	EMS[5] = 8;   // Fault_1
	EMS[6] = 9;   // Fault_2
	EMS[7] = 10;  // Fault_3
	EMS[8] = 11;  // Disabled
	EMS[9] = 12;  // Inhibited

	if (feedback_which == 1)
	{
		if (motion_mode == 0)
		{
			for (i = 0; i < num_ems; i++)
				EMS[i] += 32;
		}
		else if (motion_mode == 1)
		{
			for (i = 0; i < num_ems; i++)
				EMS[i] += 0;
		}
		else if (motion_mode == 2)
		{
			for (i = 0; i < num_ems; i++)
				EMS[i] += 64 + MDA_FILE_NO;
		}
	}
	else if (feedback_which == 0)
	{
		if (motion_mode == 0)
		{
			for (i = 0; i < num_ems; i++)
				EMS[i] += 32 + 128;
		}
		else if (motion_mode == 1)
		{
			for (i = 0; i < num_ems; i++)
				EMS[i] += 0 + 128;
		}
		else if (motion_mode == 2)
		{
			for (i = 0; i < num_ems; i++)
				EMS[i] += 64 + 128 + MDA_FILE_NO;
		}
	}
}



void MotionBaseInit()
{
	UpdateFlag = false;
	SmoothFlag = false;
	da = db = dc = dd = de = df = 0;
	
}




void SetHost(char *h)
{
	strcpy_s(IPAddress, h);
}


char* GetHost()
{
	return IPAddress;
}

void UpdateMotionBase()
{
	UpdateFlag = true;
}

void SetSmoothing(bool w)
{ 
	SmoothFlag = w; 
	cout << "Set smoothing call, bool=" << w << endl;
	
}

unsigned int swapbits(unsigned int varin)
{
	int cnt = 0;
	int base = 24;
	int varsize = 4;
	int bytesize = 8;
	unsigned int outsum = 0;

	int balance = varin;

	// Change the byte order of a 4-bit variable.
	for (int i = (varsize * bytesize) - 1; i >= 0; i--)
	{
		if ((unsigned int)pow((float)2, (float)i) <= balance)
		{
			balance -= (unsigned int)pow((float)2, (float)i);
			outsum += (unsigned int)pow((float)2, ((float)i - (float)base));

			if (balance < 0.0)
			{
				printf("Trouble!\n");
				exit(1);
			}
			else if (balance == 0)
				break;
		}

		cnt++;
		if (cnt >= 24)
			base = -24;
		else if (cnt >= 16)
			base = -8;
		else if (cnt >= 8)
			base = 8;
		else
			base = 24;
	}

	return outsum;
}

float floatSwap(float f)
{
	union
	{
		float f;
		unsigned char b[4];
	} dat1, dat2;

	dat1.f = f;
	dat2.b[3] = dat1.b[0];
	dat2.b[2] = dat1.b[1];
	dat2.b[1] = dat1.b[2];
	dat2.b[0] = dat1.b[3];

	//printf(" %u %u %u %u\n", dat1.b[0], dat1.b[1], dat1.b[2], dat1.b[3]);
	//printf(" %u %u %u %u\n", dat2.b[0], dat2.b[1], dat2.b[2], dat2.b[3]);

	return dat2.f;
}

float FloatSwap(char *value)
{
	char buffer[4];

	buffer[0] = value[3];
	buffer[1] = value[2];
	buffer[2] = value[1];
	buffer[3] = value[0];

	return *((float *)&buffer);
}


//void MotionBaseInit(char *h)
//{
//	UpdateFlag = false;
//	SmoothFlag = false;
//	da = db = dc = dd = de = df = 0;
//	fb_params();
//
//	sm = {};
//	sm_rev = {};
//
//
//	SetHost(h);
//	cout << "MotionBaseInit SetHost=" << h << endl;
//}

int MotionStart(char * host)
{

	//rd test

	//return 1;
	//dll_loaded = true;

	//if (dll_loaded)
	//{
	//	//if dll was already loaded we don't have to start motion platform again
	//	return 1;
	//}


	UpdateFlag = false;
	SmoothFlag = false;
	da = db = dc = dd = de = df = 0;
	fb_params();

	sm = {};
	sm_rev = {};

	SetHost(host);
	cout << "MotionBaseInit SetHost=" << host << endl;

	// what we return 1 if success, 0 if failure
	int answer;

	// first, set up the comm between program and mb

	SetupSockets();
	printf("sockets initialized...\n");

	// set the running flag up so the read and write threads continue
	Running = true;

	motion_mode = 2;


	dll_loaded = true;



	











	// Fire off the read and write process, or processes.
	if (rev_comm == 0)
	{
		writeThread = std::thread(WriteMotionBase);
		readThread = std::thread(ReadMotionBase);

	}
	else
	{
		readwriteThread = std::thread(ReadWriteMotionBase);
	}


	cout << "Thread was started.." << endl;



	

	//// Now send a DOF *Command* mode word
	//if (rev_comm == 0)
	//{
	//	if (motion_mode == 0)
	//		SetMotionCommand(swapbits(DOF_MODE));
	//	else if (motion_mode == 1)
	//		SetMotionCommand(swapbits(LENGTH_MODE));
	//	else if (motion_mode == 2)
	//		SetMotionCommandMDA(swapbits(MDA_MODE));
	//	
	//}

	// now wait to Engage until we're in idle state



	//rd  **************************
	//show state before any command

	printf("State at the beginning is %d...\n", State);
	Sleep(2000.);
	printf("State at the beginning_2 is %d...\n", State);
	Sleep(2000.);
	//******************************




	//rd
	while (State != EMS[1])
	{

		if (motion_mode == 0)
			SetMotionCommand(swapbits(DOF_MODE));
		else if (motion_mode == 1)
			SetMotionCommand(swapbits(LENGTH_MODE));
		else if (motion_mode == 2)
			SetMotionCommandMDA(swapbits(MDA_MODE));



		
		Sleep(1000.);
		// EXPLICITLY Change the MDA file number. (101, 102, etc.)
		if (motion_mode == 2)
			SetMotionCommandMDA(swapbits(NEW_MDA_FILE + MDA_FILE_NO));

		Sleep(1.);

	// now wait until we are engaged.
		printf("waiting for IDLE; state is %d %d...\n", State, EMS[1]);
	}
	printf("IN IDLE STATE..\n");
	
	
	// now wait until we are engaged.
	if (motion_mode == 2)
		SetMotionCommandMDA(swapbits(ENGAGE + MDA_FILE_NO));  // tmp
	else
		SetMotionCommand(swapbits(ENGAGE));

	
	
	
	while (State != EMS[3])
	{
		Sleep(100.);
		printf("waiting for ENGAGE; state is %d...\n", State);
	}
	printf("IN ENGAGED MODE..\n");



	// now we're engaged, send dof's
	if (motion_mode != 2)
		SetMotionCommand(swapbits(NEW_POSITION));
	else if (motion_mode == 2)
		SetMotionCommandMDA(swapbits(NEW_MDA + MDA_FILE_NO));




	//rd we don't need this in MDA mode  - motion_mode==2  ????
	//if (motion_mode == 0) {
	//	// now we're engaged, send dof's
	//	SetMotionCommand(swapbits(NEW_POSITION));

	//	
	//}

	// now, turn smoothing on, set smoothing values
	SetSmoothing(true);
	SetSmoothingValues((float)to_neut_sm / udf, (float)to_neut_sm / udf, (float)to_neut_sm / udf, (float)to_neut_sm / udf, (float)to_neut_sm / udf, (float)to_neut_sm / udf);
	

	// Set all dofs to API 0
	if (motion_mode == 0)
	{
		// DOF mode - sets base to neutral position.
		SetRoll(0);
		SetPitch(0);
		SetYaw(0);
		SetHeave(0);
		SetSurge(0);
		SetSway(0);
	}

	// Write an updated command word to the MB.
	UpdateMotionBase();

	cout << "#####   All set   #####" << endl;


	return 1;
}

void ShutDownMotionBase()
{


	// bring it into park
	if (motion_mode != 2)
		SetMotionCommand(swapbits(PARK));
	else if (motion_mode == 2)
		SetMotionCommandMDA(swapbits(PARK + MDA_FILE_NO));

	// wait for response
	while (State != EMS[4])
	{
		//printf("waiting ...; state is %d %d...\n", State, EMS[4]);
		cout << "waiting ...; state is " << State << EMS[4] <<endl;

		Sleep(100.);
	}
		


	// now we are in PARK mode


	
	while (State != EMS[1])
	{

		if (motion_mode == 0)
			SetMotionCommand(swapbits(DOF_MODE));
		else if (motion_mode == 1)
			SetMotionCommand(swapbits(LENGTH_MODE));
		else if (motion_mode == 2)
			SetMotionCommandMDA(swapbits(MDA_MODE));




		Sleep(1000.);

		// now wait until we are engaged.
		printf("waiting for IDLE; state is %d %d...\n", State, EMS[1]);
	}


	//Sleep(5000);

	if (rev_comm == 0)
	{
		readThread.detach();
		writeThread.detach();
	}
	else
	{
		readwriteThread.detach();
	}

	
	// kill the two threads
	Running = false;

	// Close the socket.
	closesocket(sendsocket);
}

void SetRoll(float f)
{
	float range;
	if (f>1.0) f = 1.0;
	else
		if (f<-1.0) f = -1.0;
	range = MAX_ROLL - MIN_ROLL;
	f += 1.0;//normalize it positive
	hold.a = MIN_ROLL + (f*range) / 2.0;
}

void SetPitch(float f)
{
	float range;
	if (f>1.0) f = 1.0;
	else
		if (f<-1.0) f = -1.0;
	range = MAX_PITCH - MIN_PITCH;
	f += 1.0;//normalize it positive
	hold.b = MIN_PITCH + (f*range) / 2.0;
}

void SetYaw(float f)
{
	float range;
	if (f>1.0) f = 1.0;
	else
		if (f<-1.0) f = -1.0;
	range = MAX_YAW - MIN_YAW;
	f += 1.0;//normalize it positive
	hold.e = MIN_YAW + (f*range) / 2.0;
}

void SetHeave(float f)
{
	float range;
	if (f>1.0) f = 1.0;
	else
		if (f<-1.0) f = -1.0;
	range = MAX_HEAVE - MIN_HEAVE;
	f += 1.0;//normalize it positive
	hold.c = MIN_HEAVE + (f*range) / 2.0;
}

void SetSurge(float f)
{
	float range;
	if (f>1.0) f = 1.0;
	else
		if (f<-1.0) f = -1.0;
	range = MAX_SURGE - MIN_SURGE;
	f += 1.0;//normalize it positive
	hold.d = MIN_SURGE + (f*range) / 2.0;
}

void SetSway(float f)
{
	float range;
	if (f>1.0) f = 1.0;
	else
		if (f<-1.0) f = -1.0;
	range = MAX_SWAY - MIN_SWAY;
	f += 1.0;//normalize it positive
	hold.f = MIN_SWAY + (f*range) / 2.0;
}



void SetMDA(SendMDA mda_rev)
{
	// sm_rev will be swaped in rbo()  => sm
	// sm will be sent to Motion Base
	
	
	
	
	//12/6/18
	//sm_rev = mda_rev;
	sm_rev_oryginal = mda_rev;


	/*std::cout << "SetMDA, a=" << mda_rev.a 
		<< ", b=" << mda_rev.b 
		<< ", c=" << mda_rev.c
		<< ", d=" << mda_rev.d
		<< ", e=" << mda_rev.e

		<< ", f=" << mda_rev.f
		<< ", g=" << mda_rev.g
		<< ", h=" << mda_rev.h
		<< ", i=" << mda_rev.i
		<< ", j=" << mda_rev.j
		<< ", k=" << mda_rev.k
		<< ", l=" << mda_rev.l
		<< ", m=" << mda_rev.m
		<< ", n=" << mda_rev.n
		<< ", o=" << mda_rev.o
		<<  std::endl;*/
}

//float GetRoll()
//{
//	float range;
//	float answer;
//	range = MAX_ROLL - MIN_ROLL;
//	answer = r.a - MIN_ROLL;
//	answer = (2.0*answer) / range;
//	answer -= 1.0;
//	return answer;
//}
//
//float GetPitch()
//{
//	float range;
//	float answer;
//	range = MAX_PITCH - MIN_PITCH;
//	answer = r.b - MIN_PITCH;
//	answer = (2.0*answer) / range;
//	answer -= 1.0;
//	return answer;
//}
//
//float MotionBase::GetYaw()
//{
//	float range;
//	float answer;
//	range = MAX_YAW - MIN_YAW;
//	answer = r.e - MIN_YAW;
//	answer = (2.0*answer) / range;
//	answer -= 1.0;
//	return answer;
//}
//
//float MotionBase::GetHeave()
//{
//	float range;
//	float answer;
//	range = MAX_HEAVE - MIN_HEAVE;
//	answer = r.c - MIN_HEAVE;
//	answer = (2.0*answer) / range;
//	answer -= 1.0;
//	return answer;
//}
//
//float MotionBase::GetSurge()
//{
//	float range;
//	float answer;
//	range = MAX_SURGE - MIN_SURGE;
//	answer = r.d - MIN_SURGE;
//	answer = (2.0*answer) / range;
//	answer -= 1.0;
//	return answer;
//}
//
//float MotionBase::GetSway()
//{
//	float range;
//	float answer;
//	range = MAX_SWAY - MIN_SWAY;
//	answer = r.f - MIN_SWAY;
//	answer = (2.0*answer) / range;
//	answer -= 1.0;
//	return answer;
//}

//int MotionBase::GetState()
//{
//	float si = 1. / 0.0254;
//	float os = 28.0*.0254;  // see Xform code from Moog.
//
//							// Feedback - lengths:
//	if (feedback_which == 0)
//	{
//		//printf("DOF:  R P H S Y L\n");
//		//printf("SI: %f %f %f %f %f %f\n", floatSwap(r.a), floatSwap(r.b), floatSwap(r.c), floatSwap(r.d), floatSwap(r.e), floatSwap(r.f));
//		//fprintf(fpg, "%f %f %f %f %f %f\n", floatSwap(r.a), floatSwap(r.b), floatSwap(r.c), floatSwap(r.d), floatSwap(r.e), floatSwap(r.f));
//	}
//
//	if (feedback_which == 1)
//	{
//		//printf("LENGTH:  A B C D E F\n");
//		//printf("SI: %f %f %f %f %f %f\n", floatSwap(r.a), floatSwap(r.b), floatSwap(r.c), floatSwap(r.d), floatSwap(r.e), floatSwap(r.f));
//		//fprintf(fpg, "%f %f %f %f %f %f\n", floatSwap(r.a), floatSwap(r.b), floatSwap(r.c), floatSwap(r.d), floatSwap(r.e), floatSwap(r.f));
//		//printf("US: %f %f %f %f %f %f\n", r.a*si, r.b*si, r.c*si, r.d*si, r.e*si, r.f*si);
//
//		//printf("SIoffset: %f %f %f %f %f %f\n", r.a+os, r.b+os, r.c+os, r.d+os, r.e+os, r.f+os);
//		//printf("USoffset: %f %f %f %f %f %f\n", (r.a+os)*si, (r.b+os)*si, (r.c+os)*si, (r.d+os)*si, (r.e+os)*si, (r.f+os)*si);
//	}
//
//	return 0;
//}

void ReadMotionBase()
{
	int idum = 1;
	int result = -1, res2 = -1;
	fd_set readset;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	printf("  Read process started...\n");
	int recvAddrSize = sizeof(cliAddr);

	while (Running)
	{
		FD_ZERO(&readset);
		FD_SET(sendsocket, &readset);
		result = select(idum, &readset, NULL, NULL, &tv);

		if (result > 0)
		{
			// Note: use sendsocket, with RECV address and Size!!
			res2 = recvfrom(sendsocket, (char *)&r, sizeof(Response), 0, (struct sockaddr *)&cliAddr, &recvAddrSize);
			if (res2 > 0)
				State = swapbits(r.status);
		}
	}
	printf("  Read process ending.\n");
}

//extern "C" void* start_R(void *a)
//{
//	static_cast<MotionBase*>(a)->ReadMotionBase();
//	return 0;
//}

void rbo(void)
{
	char* sval;
	float hlenn = 0.1655f;
	float hlenn2 = 0.1685;

	if (motion_mode == 0)
	{
		sval = (char *)&s_rev.a;
		s.a = FloatSwap(sval);

		sval = (char *)&s_rev.b;
		s.b = FloatSwap(sval);

		sval = (char *)&s_rev.c;
		s.c = FloatSwap(sval);

		sval = (char *)&s_rev.d;
		s.d = FloatSwap(sval);

		sval = (char *)&s_rev.e;
		s.e = FloatSwap(sval);

		sval = (char *)&s_rev.f;
		s.f = FloatSwap(sval);

		/*s.a = FloatSwap(s_rev.a);
		s.b = FloatSwap(s_rev.b);
		s.c = FloatSwap(s_rev.c);
		s.d = FloatSwap(s_rev.d);
		s.e = FloatSwap(s_rev.e);
		s.f = FloatSwap(s_rev.f); */
	}
	else if (motion_mode == 1)
	{
		sval = (char *)&hlenn;
		s.a = FloatSwap(sval);

		sval = (char *)&hlenn;
		s.b = FloatSwap(sval);

		sval = (char *)&hlenn;
		s.c = FloatSwap(sval);

		sval = (char *)&hlenn;
		s.d = FloatSwap(sval);

		sval = (char *)&hlenn;
		s.e = FloatSwap(sval);

		if (alt == 0)
		{
			sval = (char *)&hlenn2;
			//fprintf(fpgg, "%f\n", hlenn2); 
			alt = 1;
		}
		else
		{
			sval = (char *)&hlenn;
			//fprintf(fpgg, "%f\n", hlenn); 
			alt = 0;
		}
		s.f = FloatSwap(sval);
	}
	else
	{
		/*sm.a = FloatSwap(sm_rev.a);
		sm.b = FloatSwap(sm_rev.b);
		sm.c = FloatSwap(sm_rev.c);
		sm.d = FloatSwap(sm_rev.d);
		sm.e = FloatSwap(sm_rev.e);
		sm.f = FloatSwap(sm_rev.f);*/


		

		sval = (char *)&sm_rev.a;
		sm.a = FloatSwap(sval);

		sval = (char *)&sm_rev.b;
		sm.b = FloatSwap(sval);

		sval = (char *)&sm_rev.c;
		sm.c = FloatSwap(sval);

		sval = (char *)&sm_rev.d;
		sm.d = FloatSwap(sval);

		sval = (char *)&sm_rev.e;
		sm.e = FloatSwap(sval);

		sval = (char *)&sm_rev.f;
		sm.f = FloatSwap(sval);

		sval = (char *)&sm_rev.g;
		sm.g = FloatSwap(sval);

		sval = (char *)&sm_rev.h;
		sm.h = FloatSwap(sval);

		sval = (char *)&sm_rev.i;
		sm.i = FloatSwap(sval);

		sval = (char *)&sm_rev.j;
		sm.j = FloatSwap(sval);

		sval = (char *)&sm_rev.k;
		sm.k = FloatSwap(sval);

		sval = (char *)&sm_rev.l;
		sm.l = FloatSwap(sval);

		sval = (char *)&sm_rev.m;
		sm.m = FloatSwap(sval);

		sval = (char *)&sm_rev.n;
		sm.n = FloatSwap(sval);

		sval = (char *)&sm_rev.o;
		sm.o = FloatSwap(sval);

		sval = (char *)&sm_rev.p;
		sm.p = FloatSwap(sval);

		sval = (char *)&sm_rev.q;
		sm.q = FloatSwap(sval);

		sval = (char *)&sm_rev.r;
		sm.r = FloatSwap(sval);

		sval = (char *)&sm_rev.s;
		sm.s = FloatSwap(sval);


	}

}

void ReadWriteMotionBase(void)
{
	
	CDuration dur;
	double tt, begintime, endtime;

	
	int idum = 1;
	int result = -1, res2 = -1;
	fd_set readset;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;


	//12/6/18 Vibrations

	float	vibr_amplitude = 0.0f;
	float	vibr_max_amplitude = 0.0002f;
	float	vibr_delta = 1.0f;
	float	vibr_buffet = 0.0f;
	float	vibr_amplitude_factor = 1.0f;
	float	vibr_factor_minimum = 0.0f;	// we don't want to decrease vibration to zero, this is minimum value of factor
	float	vibr_factor_threshold = 16.0f;  // speed 16m/s and more -> minimum vibration, factor = vibr_factor_minimum 
	float	vehicle_speed = 0.0f;

	VibrationFrequency vibr_freq = VibrationFrequency::Low;


	printf("  ReadWrite process started...\n");
	int recvAddrSize = sizeof(cliAddr);

	while (Running)
	{

		dur.Start();



		FD_ZERO(&readset);
		FD_SET(sendsocket, &readset);
		result = select(idum, &readset, NULL, NULL, &tv);
		//rd printf("  ReadWrite TEST TEST");

		if (result > 0)
		{
			//// Note: use sendsocket, with RECV address and Size!!
			//res2 = recvfrom(sendsocket, (char *)&r, sizeof(Response), 0, (struct sockaddr *)&cliAddr, &recvAddrSize);
			//if (res2 > 0)
			//	State = swapbits(r.status);

			//// Interrupt sends packet TO Motion Base.
			////Sleep(0);
			//if (motion_mode <= 1)
			//	sendto(sendsocket, (char *)&s, sizeof(SendCommand), 0, (struct sockaddr *)&remoteSendAddr, sizeof(remoteSendAddr));
			//else
			//	sendto(sendsocket, (char *)&sm, sizeof(SendMDA), 0, (struct sockaddr *)&remoteSendAddr, sizeof(remoteSendAddr));










			// Prepare the next packet (update/smooth), for DOF mode only.
			if (motion_mode == 0)
			{
				if (UpdateFlag)
				{
					goal = hold;
					UpdateFlag = false;
				}
				if (SmoothFlag)
					Smooth();
				else
					s_rev = goal;
			}






			// vibration of the car

			vehicle_speed = sm_rev_oryginal.s; //sm_rev_oryginal.s - linear speed
			// the amplitude of vibrations (vibr_factor) decrease  as a speed increases
			vibr_amplitude_factor = 1 - (abs(vehicle_speed) / vibr_factor_threshold);	

			if (vibr_amplitude_factor< vibr_factor_minimum)
			{
				vibr_amplitude_factor = vibr_factor_minimum;
			}

			if (vibr_amplitude_factor>1)
			{
				vibr_amplitude_factor = 1;
			}

			//determine the frequency of vibration
			if (vehicle_speed > 10)	// 10m/s -> 22mph 
			{
				vibr_freq = VibrationFrequency::High;
			}
			else
			{
				vibr_freq = VibrationFrequency::Low;
			}
			 
			
			
				
				if (vibr_freq == VibrationFrequency::High)
				{
					//vibr_delta = -2.0f * vibr_delta;
					if (vibr_amplitude==1.0f)
					{
						vibr_amplitude == -1.0f;
					}
					else
					{
						vibr_amplitude == 1.0f;
					}

				}
				else
				{	//low freq
					//vibr_amplitude loop: 1.0 0.01 -1.0 -0.01 1.0 0.01 -1.0 -0.01 ...


					//old vibr_delta = -vibr_delta;

					if (vibr_amplitude == 1.0f)
					{
						vibr_amplitude = 0.01f;
					}
					else if (vibr_amplitude == 0.01f)
					{
						vibr_amplitude = -1.0f;
					}
					else if (vibr_amplitude == -1.0f)
					{
						vibr_amplitude = -0.01f;
					}
					else 
					{
						vibr_amplitude = 1.0f;
					}

				}

			
				vibr_buffet = vibr_max_amplitude * vibr_amplitude * vibr_amplitude_factor;


			// limit the value of vibr_buffet
			if (vibr_buffet>0.002) 
			{
				vibr_buffet = 0.002;
			}

			if (vibr_buffet< -0.002)
			{
				vibr_buffet = -0.002;
			}

			sm_rev = sm_rev_oryginal;

			

			sm_rev.o = sm_rev.o + vibr_buffet;


			cout << sm_rev.o << " " << vibr_buffet << endl;



			// Reverse the Byte order.
			rbo();


			// End timer.
			for (;;)
			{
				dur.Stop();
				tt = dur.GetDuration() / 1000000.0;

				if (((1.0 / (float)update_speed) - tt) <= 0.0)
				{
					float diff = ((1.0 / (float)update_speed) - tt);
					printf("tt, diff: %f %f\n", tt, diff);
					break;
				}
			}








		}
	}
	printf("  ReadWrite process ending.\n");
}

//extern "C" void* start_RW(void *a)
//{
//	static_cast<MotionBase*>(a)->ReadWriteMotionBase();
//	return 0;
//}


void WriteMotionBase(void)
{
	CDuration dur;
	double tt, begintime, endtime;

	printf("  Write process started...\n");
	while (Running)
	{
		dur.Start();

		if (motion_mode <= 1)
			sendto(sendsocket, (char *)&s, sizeof(SendCommand), 0, (struct sockaddr *)&remoteSendAddr, sizeof(remoteSendAddr));
		else
			sendto(sendsocket, (char *)&sm, sizeof(SendMDA), 0, (struct sockaddr *)&remoteSendAddr, sizeof(remoteSendAddr));

		// Prepare the next packet (update/smooth), for DOF mode only.
		if (motion_mode == 0)
		{
			if (UpdateFlag)
			{
				goal = hold;
				UpdateFlag = false;
			}
			if (SmoothFlag)
				Smooth();
			else
				s_rev = goal;
		}

		// Reverse the Byte order.
		rbo();

		// End timer.
		for (;;)
		{
			dur.Stop();
			tt = dur.GetDuration() / 1000000.0;

			if (((1.0 / (float)update_speed) - tt) <= 0.0)
			{
				float diff = ((1.0 / (float)update_speed) - tt);
				//printf("tt, diff: %f %f\n", tt, diff);
				break;
			}
		}

	}
	printf("  Write process ending.\n");
}

//extern "C" void* start_W(void *a)
//{
//	static_cast<MotionBase*>(a)->WriteMotionBase();
//	return 0;
//}

void SetupSockets()
{
	int result = -1;
	struct hostent *h;

	// Start Winsock up.  (This is a difference from Unix sockets).
	WSAData wsaData;
	int nCode;
	if ((nCode = WSAStartup(MAKEWORD(1, 1), &wsaData)) != 0)
	{
		printf("startup error.\n");
		exit(1);
	}

	/* get server IP address (no check if input is IP address or DNS name */
	h = gethostbyname(IPAddress);
	if (h == NULL)
	{
		printf("%s: unknown host\n", GetHost());
		exit(1);
	}

	printf("%s: sending data to '%s' (IP : %s) \n", "MBHOST", h->h_name, inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));

	remoteSendAddr.sin_family = h->h_addrtype;
	memcpy((char *)&remoteSendAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	remoteSendAddr.sin_port = htons(REMOTE_SERVER_SEND_PORT);

	/* socket creation */
	sendsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	printf("ss = %d\n", sendsocket);
	if (sendsocket<0)
	{
		printf("%s: cannot open socket \n", GetHost());
		exit(1);
	}

	/* Bind socket to client - use RECV port! */
	cliAddr.sin_family = AF_INET;

	cliAddr.sin_addr.s_addr = inet_addr(DCIP);  // Merovingian Second Card.
												//cliAddr.sin_addr.s_addr = INADDR_ANY;	//rd
	cliAddr.sin_port = htons(REMOTE_SERVER_RECEIVE_PORT);

	result = bind(sendsocket, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
	if (result<0)
	{
		printf("%s: cannot bind port\n", GetHost());
		exit(1);
	}

	// Verify the MB IP name and default port.
	getpeername(sendsocket, (struct sockaddr *)&remoteSendAddr, (int *)sizeof(remoteSendAddr));
	printf("Peer IP address: %s\n", inet_ntoa(remoteSendAddr.sin_addr));
	printf("Peer port      : %d\n", ntohs(remoteSendAddr.sin_port));


	//rd
	getsockname(sendsocket, (struct sockaddr *)&cliAddr, (int *)sizeof(cliAddr));
	printf("My IP address: %s\n", inet_ntoa(cliAddr.sin_addr));
	printf("My port      : %d\n", ntohs(cliAddr.sin_port));
}


void Smooth()
{
	bool Direction;//true is positive

				   //first smooth roll
	if ((goal.a - s_rev.a) >= 0)
		Direction = true;
	else
		Direction = false;
	if (Direction)
	{
		if ((goal.a - s_rev.a)>da)
			s_rev.a += da;
		else
			s_rev.a = goal.a;
	}
	else
	{
		if ((s_rev.a - goal.a)>da)
			s_rev.a -= da;
		else
			s_rev.a = goal.a;
	}

	//now pitch
	if ((goal.b - s_rev.b) >= 0)
		Direction = true;
	else
		Direction = false;
	if (Direction)
	{
		if ((goal.b - s_rev.b)>db)
			s_rev.b += db;
		else
			s_rev.b = goal.b;
	}
	else
	{
		if ((s_rev.b - goal.b)>db)
			s_rev.b -= db;
		else
			s_rev.b = goal.b;
	}

	//now heave
	if ((goal.c - s_rev.c) >= 0)
		Direction = true;
	else
		Direction = false;
	if (Direction)
	{
		if ((goal.c - s_rev.c)>dc)
			s_rev.c += dc;
		else
			s_rev.c = goal.c;
	}
	else
	{
		if ((s_rev.c - goal.c)>dc)
			s_rev.c -= dc;
		else
			s_rev.c = goal.c;
	}


	//now surge
	if ((goal.d - s_rev.d) >= 0)
		Direction = true;
	else
		Direction = false;
	if (Direction)
	{
		if ((goal.d - s_rev.d)>dd)
			s_rev.d += dd;
		else
			s_rev.d = goal.d;
	}
	else
	{
		if ((s_rev.d - goal.d)>dd)
			s_rev.d -= dd;
		else
			s_rev.d = goal.d;
	}


	//yaw
	if ((goal.e - s_rev.e) >= 0)
		Direction = true;
	else
		Direction = false;
	if (Direction)
	{
		if ((goal.e - s_rev.e)>de)
			s_rev.e += de;
		else
			s_rev.e = goal.e;
	}
	else
	{
		if ((s_rev.e - goal.e)>de)
			s_rev.e -= de;
		else
			s_rev.e = goal.e;
	}


	//sway
	if ((goal.f - s_rev.f) >= 0)
		Direction = true;
	else
		Direction = false;
	if (Direction)
	{
		if ((goal.f - s_rev.f)>df)
			s_rev.f += df;
		else
			s_rev.f = goal.f;
	}
	else
	{
		if ((s_rev.f - goal.f)>df)
			s_rev.f -= df;
		else
			s_rev.f = goal.f;
	}
}
