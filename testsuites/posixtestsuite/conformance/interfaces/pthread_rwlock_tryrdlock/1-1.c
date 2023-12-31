/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_tryrdlock(pthread_rwlock_t *pthread_rwlock_tryrdlock_1_1_rwlock)
 *
 *	The pthread_rwlock_tryrdlock( ) function shall apply a read lock as in the 
 *	pthread_rwlock_rdlock( )
 *	function, with the exception that the function shall fail if the 
 *	equivalent pthread_rwlock_rdlock( )
 *	call would have blocked the calling thread. In no case shall the 
 *	pthread_rwlock_tryrdlock( )
 *	function ever block; it always either acquires the lock or fails and 
 *	returns immediately.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_tryrdlock_1_1_rwlock' with pthread_rwlock_init()
 * 2.  Main thread read lock 'pthread_rwlock_tryrdlock_1_1_rwlock'
 * 3.  Create a child thread, the thread read lock 'pthread_rwlock_tryrdlock_1_1_rwlock' using pthread_rwlock_tryrdlock(), should success
 * 4.  Main thread unlock 'pthread_rwlock_tryrdlock_1_1_rwlock'
 * 5.  Main thread write lock 'pthread_rwlock_tryrdlock_1_1_rwlock'
 * 6.  Create child thread to read lock 'pthread_rwlock_tryrdlock_1_1_rwlock' with pthread_rwlock_tryrdlock(), should not block and get EBUSY
 * 7.  Main thread unlock 'pthread_rwlock_tryrdlock_1_1_rwlock'
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t pthread_rwlock_tryrdlock_1_1_rwlock;
static int pthread_rwlock_tryrdlock_1_1_thread_state; 

/* pthread_rwlock_tryrdlock_1_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_tryrdlock_1_1_fn_rd_1(void *arg)
{ 
	int rc = 0;

	pthread_rwlock_tryrdlock_1_1_thread_state = ENTERED_THREAD;
	printf("rd_thread1: attempt pthread_rwlock_tryrdlock\n");
	rc = pthread_rwlock_tryrdlock(&pthread_rwlock_tryrdlock_1_1_rwlock);
	if(rc != 0)
	{
		printf("Test fail at pthread_rwlock_tryrdlock(): error code: %d\n", rc);
		return PTS_FAIL ;
	}
	printf("rd_thread1: acquired read lock\n");
	printf("rd_thread1: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_tryrdlock_1_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_tryrdlock_1_1_thread_state = EXITING_THREAD;
	return NULL;
}

static void* pthread_rwlock_tryrdlock_1_1_fn_rd_2(void *arg)
{ 
	int ret;
	pthread_rwlock_tryrdlock_1_1_thread_state = ENTERED_THREAD;
	printf("rd_thread2: attempt pthread_rwlock_tryrdlock\n");
	ret = pthread_rwlock_tryrdlock(&pthread_rwlock_tryrdlock_1_1_rwlock);
	if(ret != EBUSY)
	{
		printf("Test FAILED: expected EBUSY, got %d\n", ret);
		return PTS_FAIL ;
	}
	
	printf("rd_thread2: Correctly got EBUSY\n");
	pthread_rwlock_tryrdlock_1_1_thread_state = EXITING_THREAD;
	return NULL;
}
 
int pthread_rwlock_tryrdlock_1_1()
{
	int cnt = 0;
	int rc = 0;

	pthread_t rd_thread1, rd_thread2;
	
	if(pthread_rwlock_init(&pthread_rwlock_tryrdlock_1_1_rwlock, NULL) != 0)
	{
		printf("main: Error at pthrad_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt pthread_rwlock_tryrdlock\n");
	/* We have no lock, this read lock should succeed */	
	rc = pthread_rwlock_tryrdlock(&pthread_rwlock_tryrdlock_1_1_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: in main()  at pthread_rwlock_tryrdlock, error code:%d\n", rc);
		return PTS_FAIL;
	}
	
	printf("main: acquired read lock\n");
	
	pthread_rwlock_tryrdlock_1_1_thread_state = NOT_CREATED_THREAD;
	printf("main: create rd_thread1\n");
	if(pthread_create(&rd_thread1, NULL, pthread_rwlock_tryrdlock_1_1_fn_rd_1, NULL) != 0)
	{
		printf("main: Error at creating rd_thread1\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */
	/* We did not expect the thread to block */
	
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_tryrdlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_tryrdlock_1_1_thread_state == ENTERED_THREAD)
	{
		/* the child thread started but blocked */
		printf("Test FAILED: rd_thread1 block at pthread_rwlock_tryrdlock()\n");
		return PTS_FAIL;
	}
	else if(pthread_rwlock_tryrdlock_1_1_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state for rd_thread1: %d\n", pthread_rwlock_tryrdlock_1_1_thread_state);
		return PTS_UNRESOLVED;
	}
		
	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_tryrdlock_1_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_unlock\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_join(rd_thread1, NULL) != 0)
	{
		printf("main: Error joining rd_thread1\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_tryrdlock_1_1_rwlock) != 0)
	{
		printf("main: Error getting write lock\n");
		return PTS_UNRESOLVED;	
	}

	printf("main: acquired write lock\n");

	pthread_rwlock_tryrdlock_1_1_thread_state = NOT_CREATED_THREAD;
	cnt = 0;
	printf("main: create rd_thread2\n");
	if(pthread_create(&rd_thread2, NULL, pthread_rwlock_tryrdlock_1_1_fn_rd_2, NULL) != 0)
	{
		printf("main: Error at creating rd_thread2\n");
		return PTS_UNRESOLVED;
	}
	
	/* we do not expect rd_thread2 to block */
	do{
		sleep(1);
	}while (pthread_rwlock_tryrdlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_tryrdlock_1_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: pthread_rwlock_trylock() should not block rd_thread2\n");
		return PTS_FAIL;
	}
	else if(pthread_rwlock_tryrdlock_1_1_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state for rd_thread2\n");
		return PTS_UNRESOLVED;
	}
	
	printf("main: unlock write lock\n");
	pthread_rwlock_tryrdlock_1_1_thread_state = NOT_CREATED_THREAD;
	if(pthread_rwlock_unlock(&pthread_rwlock_tryrdlock_1_1_rwlock) != 0)
	{
		printf("main: Error at releasing write lock\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_rwlock_destroy(&pthread_rwlock_tryrdlock_1_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
