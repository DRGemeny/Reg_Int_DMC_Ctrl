#include "time_work.h"

#ifdef _WIN32
	#include <Windows.h>
	#include <time.h>
#else
	#include <sys/time.h>
	#include <unistd.h>
#endif

double get_time(double initial_time)
{
#ifdef _WIN32
	unsigned __int64 freq,current_time;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
	return ((double)(current_time))/freq-initial_time;
#else
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return ((tv.tv_sec+(tv.tv_usec*1.0E-6))-initial_time);
#endif
}

void pause_ms(int milliSeconds)
{
#ifdef _WIN32
	Sleep(milliSeconds);
#else
	usleep(milliSeconds*1000);
#endif
}