/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_key_delete()
 *
  pthread_key_delete function shall be callable from within destructor functions.  No
  destructor functions shall be invoked by pthread_key_delete.  Any destructor function
  that may have been associated with 'pthread_key_delete_2_1_key' shall no longer be called up thread exit.
 *
 * Steps:
 * 1. Create a pthread_key_delete_2_1_key with a destructor function associated with it
 * 2. In the destructor function, call pthread_key_delete
 * 3. Verify that this can be done with no errors
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define KEY_VALUE 1000

pthread_key_t pthread_key_delete_2_1_key;
int pthread_key_delete_2_1_dest_cnt;

/* Destructor funciton */
void pthread_key_delete_2_1_dest_func(void *p)
{
	pthread_key_delete_2_1_dest_cnt++;
	/* Delete the pthread_key_delete_2_1_key and check if an error has occured */
	if(pthread_key_delete(pthread_key_delete_2_1_key) != 0)
	{
		pthread_key_delete_2_1_dest_cnt++;
	}
}

/* Thread function */
void *pthread_key_delete_2_1_a_thread_func()
{

	/* Set the value of the pthread_key_delete_2_1_key to a value */	
	if(pthread_setspecific(pthread_key_delete_2_1_key, (void *)(KEY_VALUE)) != 0)
	{
		printf("Error: pthread_setspecific() failed\n");
		pthread_exit((void*) PTS_UNRESOLVED);
	}
	
	/* The thread ends here, the destructor for the pthread_key_delete_2_1_key should now be called after this */
	pthread_exit(0);
}

int pthread_key_delete_2_1()
{
	pthread_t new_th;

	/* Inialize the destructor flag */
	pthread_key_delete_2_1_dest_cnt = 0;

	/* Create a pthread_key_delete_2_1_key with a destructor function */
	if(pthread_key_create(&pthread_key_delete_2_1_key, pthread_key_delete_2_1_dest_func) != 0)
	{
		printf("Error: pthread_key_create() failed\n");
		pthread_exit((void*) PTS_UNRESOLVED);
	}

	/* Create a thread */
	if(pthread_create(&new_th, NULL, pthread_key_delete_2_1_a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for the thread's return */
	if(pthread_join(new_th, NULL) != 0)
	{
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Check if the destructor was called and if the pthread_key_delete function was
	 * called successfully */
	if(pthread_key_delete_2_1_dest_cnt != 1)
	{
		if(pthread_key_delete_2_1_dest_cnt == 0)
		{
			printf("Error calling the pthread_key_delete_2_1_key destructor function\n");
			return PTS_UNRESOLVED;		
		} else
		{
			printf("Test FAILED: pthread_key_delete failed to be called from the destructor function\n");
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
