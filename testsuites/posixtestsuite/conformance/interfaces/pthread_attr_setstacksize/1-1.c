/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_setstacksizesize()
 * 
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr) 
 * 2.  set stacksize to attr
 * 3.  create a thread with the attr
 */

#include <pthread.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_setstacksize"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define STACKADDROFFSET 0x8000000

void *pthread_attr_setstacksize_1_1_thread_func()
{
	pthread_exit(0);
	return NULL;
}
int pthread_attr_setstacksize_1_1()
{
	pthread_t new_th;
	pthread_attr_t attr;
	size_t stack_size = PTHREAD_STACK_MIN;
	size_t ssize;
	void *saddr;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		return PTS_UNRESOLVED ;
	}
	
	/* printf("stack_size = %lu\n", stack_size); */

	if (posix_memalign (&saddr, sysconf(_SC_PAGE_SIZE), 
            stack_size) != 0)
    	{
      		perror (ERROR_PREFIX "out of memory while "
                        "allocating the stack memory");
      		return PTS_UNRESOLVED ;
    	}

	rc = pthread_attr_setstacksize(&attr, stack_size);
        if (rc != 0 ) {
                perror(ERROR_PREFIX "pthread_attr_setstacksize");
                return PTS_UNRESOLVED ;
        }

	rc = pthread_attr_getstacksize(&attr, &ssize);
        if (rc != 0 ) {
                perror(ERROR_PREFIX "pthread_attr_getstacksize");
                return PTS_UNRESOLVED ;
        }
	/* printf("stack_size = %lu\n", ssize); */

	rc = pthread_create(&new_th, &attr, pthread_attr_setstacksize_1_1_thread_func, NULL);
	if (rc !=0 ) {
		perror(ERROR_PREFIX "failed to create a thread");
                return PTS_FAIL ;
        }

	rc = pthread_join(new_th, NULL);
	if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_join");
		return PTS_UNRESOLVED ;
        }

	rc = pthread_attr_destroy(&attr);
	if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_attr_destroy");
		return PTS_UNRESOLVED ;
        }
	
	printf("Test PASSED\n");
	return PTS_PASS;
}


