#ifndef _AM_TEST_OS_H
#define _AM_TEST_OS_H

#define AMLIB_DEBUG 0


#ifdef _WIN32
#pragma warning(disable : 4996) /* Warns about sprintf not safe */


#include "..\win\hr_time.h"

#define INIT_OS_HR_TIMER(x) stopWatch stopwatch
#define OS_HR_TIMER_START() startTimer(&stopwatch)
#define OS_HR_TIMER_STOP() stopTimer(&stopwatch)
#define OS_HR_TIMER_GET_ELAP() getElapsedTime(&stopwatch)
#define OS_HR_TIME_PRINT_TIMER_RES() 
#define OS_HR_PRINT_START_TIME()
#define OS_HR_PRINT_STOP_TIME()
#define AM_MALLOC(x) malloc(x)
#define AM_FREE(x) free(x)
#define AM_VALLOC(x) malloc(x)
#define AM_VFREE(x) free(x)
#define AM_SLEEP(x) Sleep(x * 1000)

#if AMLIB_DEBUG
#define AM_DEBUGPRINT(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define AM_DEBUGPRINT(fmt, ...) /* not debugging: nothing */
#endif


#else
#ifdef _APPMEMD

/* Kernel driver */

#undef AM_DEBUGPRINT /* undef it, just in case */
#ifdef AM_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define AM_DEBUGPRINT(fmt, args...) printk( KERN_DEBUG "appmem: " fmt, ## args)
#else
/* This one for user space */
# define AM_DEBUGPRINT(fmt, args...) printf(fmt, ## args)
#endif
#else
# define AM_DEBUGPRINT(fmt, args...) /* not debugging: nothing */
#endif


#define AM_MALLOC(x) kmalloc(x, GFP_KERNEL)
#define AM_FREE(x) kfree(x)
#define AM_VALLOC(x) vmalloc(x)
#define AM_VFREE(x) vfree(x)








#else
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#define INIT_OS_HR_TIMER(x) struct timespec _tmSpec1; struct timespec _tmSpec2 
#define OS_HR_TIME_PRINT_TIMER_RES() clock_getres(CLOCK_THREAD_CPUTIME_ID, &_tmSpec1); printf("CLOCK_THREAD_CPUTIME_ID Resolution Sec=%ld Nano %ld\n", _tmSpec1.tv_sec, _tmSpec1.tv_nsec)

#define OS_HR_PRINT_START_TIME() printf("SPEC1 %ld SEC %ld NANOSEC\n", _tmSpec1.tv_sec, _tmSpec1.tv_nsec)
#define OS_HR_PRINT_STOP_TIME() printf("SPEC2 %ld SEC %ld NANOSEC\n", _tmSpec2.tv_sec, _tmSpec2.tv_nsec)


#define OS_HR_TIMER_START() clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_tmSpec1)
#define OS_HR_TIMER_STOP() clock_gettime(CLOCK_THREAD_CPUTIME_ID, &_tmSpec2)

//#define OS_HR_TIMER_GET_ELAP() (double)((double) (_tmSpec2.tv_nsec - _tmSpec1.tv_nsec) / (double)(1000 * 1000 * 1000))

#define OS_HR_TIMER_GET_ELAP() get_linux_ts_elap(&_tmSpec1, &_tmSpec2)


#define AM_MALLOC(x) malloc(x)
#define AM_FREE(x) free(x)
#define AM_VALLOC(x) malloc(x)
#define AM_VFREE(x) free(x)
#define AM_SLEEP(x) sleep(x)

#if AMLIB_DEBUG
#define AM_DEBUGPRINT(fmt, args...) printf(fmt, ## args)
#else
#define AM_DEBUGPRINT(fmt, args...) /* not debugging: nothing */
#endif





#endif



#endif




#endif
