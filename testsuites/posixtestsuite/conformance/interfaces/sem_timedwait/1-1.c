/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content 
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 * sem_timedwait shall lock the unlocked semaphore and decrement the 
 * semaphore * value by one.
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
// #include <fcntl.h>
#include <time.h>
#include "posixtest.h"


#define TEST "1-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "



int sem_timedwait_1_1() {
	sem_t mysemp;
	struct timespec ts;
	int val, sts;


        if ( sem_init (&mysemp, 0, 1) == -1 ) {
                perror(ERROR_PREFIX "sem_init");
                return PTS_UNRESOLVED;
        }

	// TODO: 支持timeout后不需要
	// ts.tv_sec = time(NULL);
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		perror("clock_gettime()");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec += 1;
    ts.tv_nsec=0;

	/* Lock Semaphore */
	sts = sem_timedwait(&mysemp, &ts);
        if ( sts == -1 ) {
		perror(ERROR_PREFIX "sem_timedwait");
		return PTS_UNRESOLVED; 
	}


	/* Value of Semaphore */
	if( sem_getvalue(&mysemp, &val) == -1 ) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	/* Checking if the value of the Semaphore decremented by one */
	if(( val == 0 ) && ( sts == 0)) {
		puts("TEST PASSED");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else { 
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
