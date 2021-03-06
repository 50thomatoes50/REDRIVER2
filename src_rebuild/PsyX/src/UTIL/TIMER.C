#ifdef _WIN32
#include <windows.h>
#else
#include <stddef.h>
#endif // _WIN32

#include "UTIL/TIMER.H"

void Util_InitHPCTimer(timerCtx_t* timer)
{
#ifdef _WIN32	
	QueryPerformanceCounter((LARGE_INTEGER*)&timer->clockStart);		// as it represents 64 bit value, it's safe to use
#else
	gettimeofday(&timer->timeStart, NULL);
#endif // _WIN32
}

double Util_GetHPCTime(timerCtx_t* timer, int reset)
{
#ifdef _WIN32
	LARGE_INTEGER curr;
	LARGE_INTEGER performanceFrequency;

	QueryPerformanceFrequency(&performanceFrequency);
	QueryPerformanceCounter(&curr);

	double value = double(curr.QuadPart - timer->clockStart) / double(performanceFrequency.QuadPart);

	if (reset)
		timer->clockStart = curr.QuadPart;
#else
	timeval curr;

	gettimeofday(&curr, NULL);

	double value = (float(curr.tv_sec - timer->timeStart.tv_sec) + 0.000001f * float(curr.tv_usec - timer->timeStart.tv_usec));

	if (reset)
		timer->timeStart = curr;
#endif // _WIN32

	return value;
}