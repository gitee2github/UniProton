/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_setprioceiling()
 *
 * It MAY fail if:
 *
 * [EINVAL] - 'attr' or 'prioceiling' is invalid.
 * [EPERM] - The caller doesn't have the privilege to perform the operation.
 *
 * Steps:
 * 1.  Initialize a pthread_mutexattr_t object with pthread_mutexattr_init()
 * 2.  Call pthread_mutexattr_setprioceiling() with an invalid prioceiling value.
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <stdlib.h>
#include "posixtest.h"

int pthread_mutexattr_setprioceiling_3_2()
{
	
	/* Make sure there is prioceiling capability. */ 
	/* #ifndef _POSIX_PRIORITY_SCHEDULING
	  fprintf(stderr,"prioceiling attribute is not available for testing\n");
	  return PTS_UNRESOLVED;	
	#endif */

	pthread_mutexattr_t mta;
	int prioceiling, ret;

	/* Set 'prioceiling' out of SCHED_FIFO boundry. */
	prioceiling = sched_get_priority_max(SCHED_FIFO);
	prioceiling++;
	
	/* Set the prioceiling to an invalid prioceiling. */
	if((ret=pthread_mutexattr_setprioceiling(&mta,prioceiling)) == 0)
	{
		printf("Test PASSED: *Note: Returned 0 instead of EINVAL when passed an invalid 'proceiling' to pthread_mutexattr_setprioceiling, but standard says 'may' fail.\n");
		return PTS_PASS;
	}

	if(ret != EINVAL)
	{
		printf("Test FAILED: Invalid return code %d. Expected EINVAL or 0.\n", ret);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
