/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */

/*
 * this test case verifies that the named semaphore has been removed after
 * calling sem_unlink.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
// #include <fcntl.h>
#include "posixtest.h"


#define TEST "2-1"
#define FUNCTION "sem_unlink"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "



int sem_unlink_2_1() {
	sem_t *mysemp;
	char semname[50];

	sprintf(semname, "/" FUNCTION "_" TEST);

	/* Initial value of Semaphore is 1 */
	mysemp = sem_open(semname, O_CREAT,0,1);
    if( mysemp == SEM_FAILED || mysemp == NULL ) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}
    int rc = sem_unlink(semname);
    if ( rc == 0 ) {
        rc = sem_wait(mysemp);
        if ( rc == -1)  {
			puts("TEST FAILED: Couldn't remove named semaphore");
			return PTS_FAIL;
		} else {
			puts("TEST PASSED");
			sem_close(mysemp);
			return PTS_PASS;
		}
	}
	puts("TEST FAILED");
	return PTS_FAIL;
}

