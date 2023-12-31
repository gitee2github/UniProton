/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test to see if timer_gettime() sets errno = EINVAL if no timers have been
 * created yet and it is called with a bogus timer ID.  Since this is a "may"
 * assertion, either way is pass.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define BOGUSTID 9999

int timer_gettime_6_1(int argc, char *argv[])
{
	timer_t tid;
	struct itimerspec its;

	tid = (timer_t) BOGUSTID;
	if (timer_gettime(tid, &its) == -1) {
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
