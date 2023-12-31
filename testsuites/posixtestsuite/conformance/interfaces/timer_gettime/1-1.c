/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that timer_gettime() sets sets itimerspec.it_value to the
 * amount of time remaining.
 * - Create and arm a timer.
 * - Call timer_gettime().
 * - Ensure the value returned is within ACCEPTABLEDELTA less than
 *   the value set.
 *
 * Signal SIGCONT will be used so that it will not affect the test if
 * the timer expires.
 * Clock CLOCK_REALTIME will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "posixtest.h"

#define TIMERSEC 5
#define ACCEPTABLEDELTA 1

int timer_gettime_1_1(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec itsset, itsget;
	int delta;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	itsset.it_interval.tv_sec = 0;
	itsset.it_interval.tv_nsec = 0;
	itsset.it_value.tv_sec = TIMERSEC;
	itsset.it_value.tv_nsec = 0;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_settime(tid, 0, &itsset, NULL) != 0) {
		perror("timer_settime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_gettime(tid, &itsget) != 0) {
		perror("timer_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	delta = itsset.it_value.tv_sec - itsget.it_value.tv_sec;

	if (delta < 0) {
		printf("FAIL:  timer_gettime() value > timer_settime()\n");
		printf("%d > %d\n", (int) itsget.it_value.tv_sec, 
				(int) itsset.it_value.tv_sec);
		return PTS_FAIL;
	}

	if (delta <= ACCEPTABLEDELTA) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("FAIL:  timer_gettime() value !~= timer_settime()\n");
		printf("%d !~= %d\n", (int) itsget.it_value.tv_sec, 
					(int) itsset.it_value.tv_sec);
		return PTS_FAIL;
	}

	printf("This code should not be executed\n");
	return PTS_UNRESOLVED;
}
