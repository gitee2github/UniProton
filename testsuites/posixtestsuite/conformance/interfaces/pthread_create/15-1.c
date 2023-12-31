/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.

 
 * This sample test aims to check the following assertion:
 *
 * If the current thread has an alternate stack, the new thread does not inherit 
 * this stack
 
 * The steps are:
 * -> Create a thread with an alternate stack.
 * -> From this thread, create another thread.
 * -> Check that the new thread does not use the same stack.

 */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <string.h>
 #include <unistd.h>

 #include <sched.h>
 #include <semaphore.h>
 #include <errno.h>
 #include <assert.h>
 #include <sys/wait.h>
 
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void pthread_create_output_init()
  * void pthread_create_output(char * string, ...)
  * 
  * Those may be used to pthread_create_output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#define STD_MAIN
#define THREAD_NAMEED pthread_create_15_1_threaded
#define RUN_MAIN pthread_create_15_1
#include "threads_scenarii.c"

/* This file will define the following objects:
 * pthread_create_scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of pthread_create_scenarii
 * pthread_create_scenar_init(): function to call before use the pthread_create_scenarii array.
 * pthread_create_scenar_fini(): function to call after end of use of the pthread_create_scenarii array.
 */


/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

void * pthread_create_15_1_teststack(void * arg)
{
	int ret=0;
	*(int **)arg = &ret;
	return NULL;
}

/* Thread function */
void * pthread_create_15_1_threaded(void * arg)
{
	int ret;
	int * child_stack;
	pthread_t gchild;
	
	int sz = sysconf(_SC_THREAD_STACK_MIN);
	
	if (pthread_create_scenarii[pthread_create_sc].bottom != NULL)
	{
		#if VERBOSE > 1 
		pthread_create_output("Processing test\n");
		#endif
		
		/* Create a new thread and get a location inside its stack */
		ret = pthread_create(&gchild, NULL, pthread_create_15_1_teststack, &child_stack);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to create a thread with default attribute"); }
		
		ret = pthread_join(gchild, NULL);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to join the test thread");  }
		
		/* Check the new thread stack location was outside of the current thread location */
		/* We convert all the @ to longs */
		#if VERBOSE > 4
		pthread_create_output("Current stack : %p -> %p\n", pthread_create_scenarii[pthread_create_sc].bottom, sz + (long)pthread_create_scenarii[pthread_create_sc].bottom);
		pthread_create_output("Child location: %p\n", child_stack);
		#endif
		
		if (  (((long)pthread_create_scenarii[pthread_create_sc].bottom) < ((long)child_stack)) 
		   && (((long)child_stack) < (((long)pthread_create_scenarii[pthread_create_sc].bottom) + sz)))
	   	{
			FAILED("The new thread inherited th alternate stack from its parent"); 
		}
	}
	
	/* Signal we're done (especially in case of a detached thread) */
	do { ret = sem_post(&pthread_create_scenarii[pthread_create_sc].sem); }
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1)  {  UNRESOLVED(errno, "Failed to wait for the semaphore");  }
	
	/* return */
	return arg;
}


