/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_cancel
 * Shall request that 'thread' shall be canceled.  The target thread's 
 * cancelability state and type determines when the cancelation takes
 * effect.
 * 
 * Test when a thread is PTHREAD_CANCEL_DISABLE
 *  
 * STEPS:
 * 1. Create a thread.
 * 2. In the thread function, set the state to
 *    PTHREAD_CANCEL_DISABLE
 * 3. Setup a cleanup handler for the thread.
 * 4. Send out a thread cancel request to the new thread
 * 5. If the cancel request was honored, the cleanup handler would have
 *    been executed and the test will fail.
 * 6. If not, the thread will continue until the end of execution and exit
 *    correctly, therefore passing the test. 
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

# define INTHREAD 0 	/* Control going to or is already for Thread */
# define INMAIN 1	/* Control going to or is already for Main */

int pthread_cancel_1_2_sem1;		/* Manual semaphore */
int pthread_cancel_1_2_cleanup_flag;

/* Cleanup function that the thread executes when it is canceled.  So if
 * pthread_cancel_1_2_cleanup_flag is -1, it means that the thread was canceled, meaning
 * the test will fail. */
void pthread_cancel_1_2_a_cleanup_func(void *unused)
{
	pthread_cancel_1_2_cleanup_flag=-1;
	return;
}

/* Function that the thread executes upon its creation */
void *pthread_cancel_1_2_a_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	pthread_cleanup_push(pthread_cancel_1_2_a_cleanup_func,NULL);

	/* Indicate to main() that the thread has been created. */	
	pthread_cancel_1_2_sem1=INMAIN;

	/* Wait until main() has sent out a cancel request, meaning until it
	 * sets pthread_cancel_1_2_sem1==INMAIN.  Sleeping for 3 secs. to give time for the
	 * cancel request to be sent and processed. */ 
	while(pthread_cancel_1_2_sem1==INMAIN)
		sleep(1);

	/* Should reach here if the thread correctly ignores the cancel
	 * request. */
	pthread_cleanup_pop(0);
	pthread_cancel_1_2_cleanup_flag=1;
	pthread_exit(0);
	return NULL;
}

int pthread_cancel_1_2()
{
	pthread_t new_th;
		
	/* Initializing values */
	pthread_cancel_1_2_sem1=INTHREAD;
	pthread_cancel_1_2_cleanup_flag=0;
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, pthread_cancel_1_2_a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Make sure thread is created before we cancel it. (wait for 
	 * pthread_cancel_1_2_a_thread_func() to set pthread_cancel_1_2_sem1=1.) */
	while(pthread_cancel_1_2_sem1==INTHREAD)
		sleep(1);

	if(pthread_cancel(new_th) != 0) 
	{
		perror("Error sending cancel request\n");
		return PTS_UNRESOLVED;
	}

	/* Indicate to the thread function that the thread cancel request
	 * has been sent to it. */
	pthread_cancel_1_2_sem1=INTHREAD;
	
	/* Wait for thread to return. (i.e. either canceled incorrectly and
	 * called the cleanup function, or exited normally at the end of 
	 * thread execution like it should do. */
	while(pthread_cancel_1_2_cleanup_flag==0)
		sleep(1);

	/* This means that the cleanup function was called, meaning the
	 * thread was canceled rather than ignored the cancel request. */
	if(pthread_cancel_1_2_cleanup_flag <= 0)
	{
		printf("Test FAILED\n");
		return PTS_FAIL;
	}	
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


