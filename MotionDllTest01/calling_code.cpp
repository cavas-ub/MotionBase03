


#define PI 3.141592654

#include <math.h>



#include "mb.h"
#include "calling_code.h"

int main()
{
  FILE *fp;
  int i, idum;
  int shts = 1000;
  int cnt = 1;
  char MBIP[32];
  float fdum, sl_time;
  double A, omeg, freq, quant, quant2, time;
  double duration = 20.0;
  double hertz = 60.0;
  double tt = 0.;
  /*struct timeval t1, t3;
  struct timezone t2;*/

  // Initialize base.
  printf("Step 1:  Initializing.\n");
  sprintf(MBIP, "128.127.55.120");     // MB IP address.

  //sprintf(MBIP, "192.168.0.19");

  //sprintf(MBIP, "localhost");


  //MotionBase mb(MBIP);
  MotionBaseInit(MBIP);

  
  //mb.Initialize();
  MotionStart();

  // Slowly proceed to initial DOF's.  (60 Hz.)
  printf("Step 2: Set DOF's to initial position.\n");
  // This is "neutral" for all DOF's, including HEAVE.
  SetSmoothing(true);
  SetHeave(0);
  SetPitch(0);
  SetSurge(0);
  SetRoll(0);
  SetYaw(0);
  SetSway(0);
  UpdateMotionBase();

  // Process ride motion. (60 Hz.)
  printf("Step 3: Process Ride motion.\n");
  A = 0.25;
  freq = 0.5;
  omeg = 2*PI*freq;

  SetSmoothing(false);
  i = 0;
  for(;;)
    {
      // Start time.
      //gettimeofday(&t1, &t2);
     

	  double begin = startTimer();

      time = (double)((double)i/hertz);
      if(time > duration)
	break;

      quant = A*sin(omeg*time);
      quant2=A*cos(omeg*time);

      // Remember - motions are [-1, 1].
      SetHeave(0.5);
      SetPitch(quant);
      SetSurge(0);
      SetRoll(quant2);
      SetYaw(0);
      SetSway(0);
      
      if(cnt == (int)hertz)
	{
	  printf("%d %f %f\n", i, time, quant);
	  cnt = 0;
	}
      else
	cnt++;
      
      Update();
      
      // Sleep to maintain "h ertz"  framerate.
      for(;;)
	{
	  // End time.
	  //gettimeofday(&t3, &t2);

	  //tt = (double)(t3.tv_sec - t1.tv_sec) + (double)(t3.tv_usec - t1.tv_usec)/1000000.;		//in seconds

		  double tt = endTimer(begin);

	  
	  if(((1.0/hertz) - tt) <= 0.0)
	    break;
	}

      i++;
    }
  
  // Shut down base.
  printf("Step 4: Shut down MB.\n");
  ShutDownMotionBase();

  return 0;
}



bool isHighResTimerSupported(void)
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	printf("High Res Performance Counter Frequency: %ul\n", frequency.QuadPart);



	return frequency.QuadPart != 0;
}



long long startTimer(void)
{
	LARGE_INTEGER retVal;
	QueryPerformanceCounter(&retVal);
	return retVal.QuadPart;
}



double endTimer(long long const & begin)
{
	LARGE_INTEGER end, frequency;
	QueryPerformanceCounter(&end);
	QueryPerformanceFrequency(&frequency);

	return (double)(end.QuadPart - begin) / (double)frequency.QuadPart;
}
