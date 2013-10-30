#ifndef _AM_TEST_OS_H
#define _AM_TEST_OS_H

#ifdef _WIN32
#include "..\win\hr_time.h"

#define INIT_OS_HR_TIMER(x) stopWatch stopwatch
#define OS_HR_TIMER_START() startTimer(&stopwatch)
#define OS_HR_TIMER_STOP() stopTimer(&stopwatch)
#define OS_HR_TIMER_GET_ELAP() getElapsedTime(&stopwatch)
#define OS_HR_TIME_PRINT_TIMER_RES() 
#define OS_HR_PRINT_START_TIME()
#define OS_HR_PRINT_STOP_TIME()

#else
#include <time.h>

#define INIT_OS_HR_TIMER(x) struct timespec _tmSpec1; struct timespec _tmSpec2 
#define OS_HR_TIME_PRINT_TIMER_RES() clock_getres(CLOCK_THREAD_CPUTIME_ID, &_tmSpec1); printf("CLOCK_THREAD_CPUTIME_ID Resolution Sec=%ld Nano %ld\n", _tmSpec1.tv_sec, _tmSpec1.tv_nsec)

#define OS_HR_PRINT_START_TIME() printf("SPEC1 %ld SEC %ld NANOSEC\n", _tmSpec1.tv_sec, _tmSpec1.tv_nsec)
#define OS_HR_PRINT_STOP_TIME() printf("SPEC2 %ld SEC %ld NANOSEC\n", _tmSpec2.tv_sec, _tmSpec2.tv_nsec)


#define OS_HR_TIMER_START() clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_tmSpec1)
#define OS_HR_TIMER_STOP() clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_tmSpec2)
#define OS_HR_TIMER_GET_ELAP() (double)((double) (_tmSpec2.tv_nsec - _tmSpec1.tv_nsec) / (double)(1000 * 1000 * 1000))


#endif




#endif
