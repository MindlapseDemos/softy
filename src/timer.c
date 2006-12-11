#if defined(unix) || defined(__unix__)
#include <time.h>
#include <sys/time.h>
#else 
#include <windows.h>
#endif	/* unix */

#include "timer.h"


unsigned long get_msec(void)
{
#if defined(__unix__) || defined(unix)
	static struct timeval timeval, first_timeval;
	
	gettimeofday(&timeval, 0);

	if(first_timeval.tv_sec == 0) {
		first_timeval = timeval;
		return 0;
	}
	return (timeval.tv_sec - first_timeval.tv_sec) * 1000 + (timeval.tv_usec - first_timeval.tv_usec) / 1000;
#else
	return GetTickCount();
#endif	/* __unix__ */

}
