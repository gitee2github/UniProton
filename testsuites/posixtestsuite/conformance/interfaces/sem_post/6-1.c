/*
 *
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * sem_post() can be called from within a signal catching function
 *
 * 1) Set up a signal catching function for SIGALRM.
 * 2) Set up a semaphore
 * 3) Call alarm(1)
 * 4) Call sempost() from within the sem_post_6_1_handler for SIGALRM
 * 5) If sempost() succeeds (verified if val is incremented by 1),
 *    test passes.  Otherwise, failure.
 */


#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
// #include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include "posixtest.h"


#define TEST "6-1"
#define FUNCTION "sem_post"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define SEMINITVAL 0 //initial value of semaphore = 0

sem_t *sem_post_6_1_sighdl;

void sem_post_6_1_handler(int signo)
{
	if( sem_post(sem_post_6_1_sighdl) == -1 ) {
		perror(ERROR_PREFIX "sem_post");
		return PTS_UNRESOLVED ; 
	}
}

int sem_post_6_1() {
	char semname[50];
	struct sigaction act;
	int val;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	sem_post_6_1_sighdl = sem_open(semname, O_CREAT, 0777, SEMINITVAL);

	act.sa_handler=sem_post_6_1_handler;
	act.sa_flags=0;

	if (sigemptyset(&act.sa_mask) == -1) {
		perror("Error calling sigemptyset\n");
		return PTS_UNRESOLVED;
	}
	if (sigaction(SIGALRM, &act, 0) == -1) {
		perror("Error calling sigaction\n");
		return PTS_UNRESOLVED;
	}

	if( sem_post_6_1_sighdl == SEM_FAILED || sem_post_6_1_sighdl == NULL ) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	alarm(1);
	sleep(2);

	/* Checking if the value of the Semaphore incremented by one */
	if( sem_getvalue(sem_post_6_1_sighdl, &val) == -1 ) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}
	if (val != SEMINITVAL+1) {
#ifdef DEBUG
		printf("semaphore value was not incremented\n");
#endif
		printf("TEST FAILED\n");
		return PTS_FAIL;
	}

#ifdef DEBUG
	printf("semaphore value was %d\n", val);
#endif

	printf("TEST PASSED\n");
	sem_close(sem_post_6_1_sighdl);
	sem_unlink(semname);
	return PTS_PASS;
}

