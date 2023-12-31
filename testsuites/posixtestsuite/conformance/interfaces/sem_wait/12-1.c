/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content 
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 * sem_trywait shall try to lock the locked semaphore and decrement 
 * the semaphore value by one.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
// #include <fcntl.h>
#include "posixtest.h"


#define TEST "12-1"
#define FUNCTION "sem_trywait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "



int sem_wait_12_1() {
	sem_t *mysemp;
	char semname[50];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/* Initial value of Semaphore is 1 */
	mysemp = sem_open(semname, O_CREAT, 0777, 0);
	if( mysemp == SEM_FAILED || mysemp == NULL ) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}


	/* Try to Lock Semaphore by sem_trywait*/
	if( sem_trywait(mysemp) == -1 ) {
		puts("TEST PASSED");
		sem_unlink(semname);
		sem_close(mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED: Semaphore locked when it shouldn't");
		return PTS_FAIL;
	}
}

