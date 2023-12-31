/*   
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_attr_setinheritsched() sets the inheritance of 
 * policy and priority.

 * Steps:
 * 1.  Create a pthread_attr struct using pthread_attr_init
 * 2.  Set the inherit sched in it by calling pthread_attr_setinheritsched
 * 3.  Change main's priority and policy
 * 4.  Create a new thread.
 * 5.  Check that it has correct priority and policy
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "4-1"
#define AREA "scheduler"
#define ERROR_PREFIX "unexpected error: " AREA " " TEST ": "

#define PRIORITY 20
#define POLICY SCHED_FIFO

/* Flags that the threads use to indicate events */
int pthread_attr_setinheritsched_2_3_policy_correct = -1;
int pthread_attr_setinheritsched_2_3_priority_correct = -1;

void *pthread_attr_setinheritsched_2_3_thread(void *tmp)
{
	struct sched_param    param;
	int                   policy;
	int                   rc = 0;

	rc = pthread_getschedparam(pthread_self(), &policy, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		return PTS_UNRESOLVED ;
	}
	if(policy == POLICY) {
		pthread_attr_setinheritsched_2_3_policy_correct = 1;
	}
	if(param.sched_priority == PRIORITY) {
		pthread_attr_setinheritsched_2_3_priority_correct = 1;
	}

	return NULL;
}

int pthread_attr_setinheritsched_2_3()
{
	pthread_attr_t          attr;
	pthread_t               thread_id;
	struct sched_param      param;
	int                     rc = 0;

	param.sched_priority = PRIORITY;

	rc = pthread_attr_init(&attr);
	if(rc != 0) {	
		printf(ERROR_PREFIX "pthread_attr_init\n");
		return PTS_UNRESOLVED ;
	}

	rc = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setinheritsched\n");
		return PTS_UNRESOLVED ;
	}

	rc = pthread_setschedparam(pthread_self(), POLICY, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_setschedparam\n");
		return PTS_UNRESOLVED ;
	}

	rc = pthread_create(&thread_id, &attr, pthread_attr_setinheritsched_2_3_thread, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		return PTS_UNRESOLVED ;
	}

	rc = pthread_join(thread_id, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_join\n");
		return PTS_UNRESOLVED ;
	}

	if((pthread_attr_setinheritsched_2_3_priority_correct != 1) || (pthread_attr_setinheritsched_2_3_policy_correct != 1)) {
		printf("Test FAILED\n");
		return PTS_FAIL ;
	}

	printf("Test PASS\n");
	return PTS_PASS ;
}
