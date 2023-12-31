/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.


* This sample test aims to check the following assertion:
*
*  sem_init returns -1 and sets errno to ENOSPC if the system lacks a resource
* or SEM_NSEMS_MAX has been reached.


* The steps are:
* -> Try and sem_init SEM_NSEMS_MAX semaphores.
* -> Try and sem_init an additional semaphore.

* The test fails if the last creation does not return an error.

*/


/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <errno.h>

/******************************************************************************/
/***************************   Test framework   *******************************/
/******************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c" 
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);  
 *    where descr is a description of the error and ret is an int 
 *   (error code for example)
 * FAILED(descr);
 *    where descr is a short text saying why the test has failed.
 * PASSED();
 *    No parameter.
 * 
 * Both three macros shall terminate the calling process.
 * The testcase shall not terminate in any other maneer.
 * 
 * The other file defines the functions
 * void sem_init_output_init()
 * void sem_init_output(char * string, ...)
 * 
 * Those may be used to sem_init_output information.
 */

/******************************************************************************/
/**************************** Configuration ***********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/


/* The main test function. */
int sem_init_7_1( int argc, char * argv[] )
{
	int ret, i;
	sem_t *sems;
	sem_t sem_last;

	long max;

	/* Initialize sem_init_output */
	sem_init_output_init();

	max = sysconf( _SC_SEM_NSEMS_MAX );

	if ( max <= 0 )
	{
		sem_init_output( "sysconf( _SC_SEM_NSEMS_MAX ) = %ld\n", max );
		UNTESTED( "There is no constraint on SEM_NSEMS_MAX" );
	}

	sems = ( sem_t * ) calloc( max, sizeof( sem_t ) );

	if ( sems == NULL )
	{
		UNRESOLVED( errno, "Failed to alloc space" );
	}


	for ( i = 0; i < max; i++ )
	{
		ret = sem_init( &sems[ i ], 0, 0 );

		if ( ret != 0 )
		{
			sem_init_output( "sem_init failed to initialize the %d nth semaphore.\n", i );
			sem_init_output( "Tryed to initialize %ld.\n", max );
			sem_init_output( "Error is %d: %s\n", errno, strerror( errno ) );

			for ( ; i > 0; i-- )
				sem_destroy( &sems[ i - 1 ] );

			free( sems );

			PASSED;
		}
	}

	ret = sem_init( &sem_last, 0, 1 );

	if ( ret == 0 )
	{
		FAILED( "We were able to sem_init more than SEM_NSEMS_MAX semaphores" );
	}

	if ( errno != ENOSPC )
	{
		sem_init_output( "Error is %d: %s\n", errno, strerror( errno ) );
	}

	for ( i = 0; i < max; i++ )
		sem_destroy( &sems[ i ] );

	free( sems );


	/* Test passed */
#if VERBOSE > 0

	sem_init_output( "Test passed\n" );

#endif

	PASSED;
}


