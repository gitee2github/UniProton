/* 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that sched_get_priority_max() returns the maximum value on
 * success for SCHED_RR policy.
 */
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include "posixtest.h"

int sched_get_priority_max_1_1(int argc, char **argv)
{	       
	int result = -1;
	
	result = sched_get_priority_max(SCHED_RR);
	
	if(result != -1 && errno == 0 ) {
		printf("The maximum priority for policy SCHED_RR is %i.\n",
		       result);
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		perror("An error occurs");
		return PTS_FAIL;
	}
		
	printf("This code should not be executed.\n");
        return PTS_UNRESOLVED;
}


