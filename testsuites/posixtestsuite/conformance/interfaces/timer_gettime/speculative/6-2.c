/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test to see if timer_gettime() sets errno = EINVAL timerid != a timer ID
 * created via timer_create().  [Try to set timerid to a timer ID
 * created + 1.]  Since this assertion is a "may," either way is a pass.
 *
 * For this test, signal SIGCONT will be used, clock CLOCK_REALTIME
 * will be used.
 */

#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

int timer_gettime_6_2(int argc, char *argv[])
{
	struct sigevent ev;
	timer_t tid;
	struct itimerspec its;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGCONT;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
		perror("timer_create() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if (timer_gettime(tid+1, &its) == -1) {
		if (EINVAL == errno) {
			printf("fcn returned -1 and errno==EINVAL\n");
			return PTS_PASS;
		} else {
			printf("fcn returned -1 but errno!=EINVAL\n");
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	printf("fcn did not return -1\n");
	return PTS_PASS;
}
