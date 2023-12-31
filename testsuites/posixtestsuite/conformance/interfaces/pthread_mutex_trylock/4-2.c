/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
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
 * If the mutex was already locked, the call returns EBUSY immediatly.

 * The steps are:
 * -> Set a timeout.
 * -> For each kind of mutex,
 *   -> Lock the mutex.
 *   -> create a new child (either thread or process)
 *      -> the new child trylock the mutex. It shall fail.
 *   -> undo everything.
 */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We need the XSI extention for the mutex attributes
   and the mkstemp() routine */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
 /********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdarg.h>
 #include <stdio.h>
 #include <stdlib.h> 
 #include <unistd.h>

 #include <errno.h>
 #include <sys/wait.h>
 #include <sys/mman.h>
 #include <string.h>
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void pthread_mutex_trylock_output_init()
  * void pthread_mutex_trylock_output(char * string, ...)
  * 
  * Those may be used to pthread_mutex_trylock_output information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
typedef struct 
{
	pthread_mutex_t mtx;
	int status; /* error code */
} pthread_mutex_trylock_4_2_testdata_t;

struct _pthread_mutex_trylock_4_2_scenar
{
	int m_type; /* Mutex type to use */
	int m_pshared; /* 0: mutex is process-private (default) ~ !0: mutex is process-shared, if supported */
	int fork; /* 0: Test between threads. ~ !0: Test across processes, if supported (mmap) */
	char * descr; /* Case description */
}
pthread_mutex_trylock_4_2_scenarii[] =
{
	 {PTHREAD_MUTEX_DEFAULT,    0, 0, "Default mutex"}
#ifndef WITHOUT_XOPEN
	,{PTHREAD_MUTEX_NORMAL,     0, 0, "Normal mutex"}
	,{PTHREAD_MUTEX_ERRORCHECK, 0, 0, "Errorcheck mutex"}
	,{PTHREAD_MUTEX_RECURSIVE,  0, 0, "Recursive mutex"}
#endif

	,{PTHREAD_MUTEX_DEFAULT,    1, 0, "Pshared mutex"}
#ifndef WITHOUT_XOPEN
	,{PTHREAD_MUTEX_NORMAL,     1, 0, "Pshared Normal mutex"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, 0, "Pshared Errorcheck mutex"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, 0, "Pshared Recursive mutex"}
#endif

	,{PTHREAD_MUTEX_DEFAULT,    1, 1, "Pshared mutex across processes"}
#ifndef WITHOUT_XOPEN
	,{PTHREAD_MUTEX_NORMAL,     1, 1, "Pshared Normal mutex across processes"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, 1, "Pshared Errorcheck mutex across processes"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, 1, "Pshared Recursive mutex across processes"}
#endif
};
#define NSCENAR (sizeof(pthread_mutex_trylock_4_2_scenarii)/sizeof(pthread_mutex_trylock_4_2_scenarii[0]))

/* The test function will only perform a trylock operation then return. */
void * pthread_mutex_trylock_4_2_tf(void * arg)
{
	pthread_mutex_trylock_4_2_testdata_t * td = (pthread_mutex_trylock_4_2_testdata_t *)arg;
	
	td->status = pthread_mutex_trylock(&(td->mtx));
	
	if (td->status == 0)
	{
		int ret;
		
		ret = pthread_mutex_unlock(&(td->mtx));
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to unlock a locked semaphore");  }
	}
	
	return NULL;
}

/* Main entry point. */
int pthread_mutex_trylock_4_2(int argc, char * argv[])
{
	int ret;
	int sc;
	pthread_mutexattr_t ma;
	
	pthread_mutex_trylock_4_2_testdata_t * td;
	pthread_mutex_trylock_4_2_testdata_t alternativ;
	
	int do_fork;
	
	pid_t child_pr=0, chkpid;
	int status;
	pthread_t child_th;
	
	long pshared, mf;
	
	/* Initialize pthread_mutex_trylock_output */
	pthread_mutex_trylock_output_init();
	
	/* Initialize the timeout */
	alarm(30);
	
	/* Test system abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	mf =sysconf(_SC_MAPPED_FILES);
	
	#if VERBOSE > 0
	pthread_mutex_trylock_output("Test starting\n");
	pthread_mutex_trylock_output("System abilities:\n");
	pthread_mutex_trylock_output(" TSH : %li\n", pshared);
	pthread_mutex_trylock_output(" MF  : %li\n", mf);
	if ((mf < 0) || (pshared < 0))
		pthread_mutex_trylock_output("Process-shared attributes won't be tested\n");
	#endif

	#ifdef WITHOUT_XOPEN
	#if VERBOSE > 0
	pthread_mutex_trylock_output("As XSI extension is disabled, we won't test the feature across process\n");
	#endif
	mf = -1;
	#endif
	
/**********
 * Allocate space for the testdata structure
 */
	if (mf < 0)
	{
		/* Cannot mmap a file (or not interested in this), we use an alternative method */
		td = &alternativ;
		pshared = -1; /* We won't do this testing anyway */
		#if VERBOSE > 0
		pthread_mutex_trylock_output("Testdata allocated in the process memory.\n");
		#endif
	}
	#ifndef WITHOUT_XOPEN
	else
	{
		/* We will place the test data in a mmaped file */
		char filename[] = "/tmp/mutex_trylock_4-2-XXXXXX";
		size_t sz;
		void * mmaped;
		int fd;
		char * tmp;
		
		/* We now create the temp files */
		fd = mkstemp(filename);
		if (fd == -1)
		{ UNRESOLVED(errno, "Temporary file could not be created"); }
		
		/* and make sure the file will be deleted when closed */
		unlink(filename);
		
		#if VERBOSE > 1
		pthread_mutex_trylock_output("Temp file created (%s).\n", filename);
		#endif
		
		sz= (size_t)sysconf(_SC_PAGESIZE);
		
		tmp = calloc(1, sz);
		if (tmp == NULL)
		{ UNRESOLVED(errno, "Memory allocation failed"); }
		
		/* Write the data to the file.  */
		if (write (fd, tmp, sz) != (ssize_t) sz)
		{ UNRESOLVED(sz, "Writting to the file failed"); }
		
		free(tmp);
		
		/* Now we can map the file in memory */
		mmaped = mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (mmaped == MAP_FAILED)
		{ UNRESOLVED(errno, "mmap failed"); }
		
		td = (pthread_mutex_trylock_4_2_testdata_t *) mmaped;
		
		/* Our datatest structure is now in shared memory */
		#if VERBOSE > 1
		pthread_mutex_trylock_output("Testdata allocated in shared memory.\n");
		#endif
	}
	#endif
	
/**********
 * For each test scenario, initialize the attributes and other variables.
 * Do the whole thing for each time to test.
 */
	for ( sc=0; sc < NSCENAR ; sc++)
	{
		#if VERBOSE > 1
		pthread_mutex_trylock_output("[parent] Preparing attributes for: %s\n", pthread_mutex_trylock_4_2_scenarii[sc].descr);
		#endif
		/* set / reset everything */
		do_fork=0;
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to initialize the mutex attribute object");  }
		
		#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, pthread_mutex_trylock_4_2_scenarii[sc].m_type);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set mutex type");  }
		#if VERBOSE > 1
		pthread_mutex_trylock_output("[parent] Mutex type : %i\n", pthread_mutex_trylock_4_2_scenarii[sc].m_type);
		#endif
		#endif
			
		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (pthread_mutex_trylock_4_2_scenarii[sc].m_pshared != 0))
		{
			ret = pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set the mutex process-shared");  }
			#if VERBOSE > 1
			pthread_mutex_trylock_output("[parent] Mutex is process-shared\n");
			#endif
		}
		#if VERBOSE > 1
		else {
			pthread_mutex_trylock_output("[parent] Mutex is process-private\n");
		}
		#endif
			
		/* Tell whether the test will be across processes */
		if ((pshared > 0) && (pthread_mutex_trylock_4_2_scenarii[sc].fork != 0))
		{
			do_fork = 1;
			#if VERBOSE > 1
			pthread_mutex_trylock_output("[parent] Child will be a new process\n");
			#endif
		}
		#if VERBOSE > 1
		else {
			pthread_mutex_trylock_output("[parent] Child will be a new thread\n");
		}
		#endif
	
/**********
 * Initialize the pthread_mutex_trylock_4_2_testdata_t structure with the previously defined attributes 
 */
		/* Initialize the mutex */
		ret = pthread_mutex_init(&(td->mtx), &ma);
		if (ret != 0)
		{  UNRESOLVED(ret, "[parent] Mutex init failed");  }
		
		/* Initialize the other datas from the test structure */
		td->status=0;
		
/**********
 * Proceed to the actual testing 
 */
		/* Trylock the mutex twice before creating children */
		ret = pthread_mutex_lock(&(td->mtx));
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to lock the mutex");  }
			
		/* Create the children */
		if (do_fork != 0)
		{
			/* We are testing across processes */
			child_pr = fork();
			if (child_pr == -1)
			{  UNRESOLVED(errno, "[parent] Fork failed");  }
			
			if (child_pr == 0)
			{
				#if VERBOSE > 3
				pthread_mutex_trylock_output("[child] Child process is starting...\n");
				#endif
				
				if (pthread_mutex_trylock_4_2_tf((void *)td) != NULL)
				{
					UNRESOLVED( -1, "[child] Got an unexpected return value from test function");
				}
				else
				{
					/* We cannot use the PASSED macro here since it would terminate the pthread_mutex_trylock_output */
					exit (0);
				}
			}
			/* Only the parent process goes further */
		}
		else /* do_fork == 0 */
		{
			/* We are testing across two threads */
			ret = pthread_create(&child_th, NULL, pthread_mutex_trylock_4_2_tf, td);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to create the child thread.");  }
		}
			
		/* Wait for the child to terminate */
		if (do_fork != 0)
		{
			/* We were testing across processes */
			ret = 0;
			chkpid = waitpid(child_pr, &status, 0);
			if (chkpid != child_pr)
			{
				pthread_mutex_trylock_output("Expected pid: %i. Got %i\n", (int)child_pr, (int)chkpid);
				UNRESOLVED(errno, "Waitpid failed"); 
			}
			if (WIFSIGNALED(status))
			{ 
				pthread_mutex_trylock_output("Child process killed with signal %d\n",WTERMSIG(status)); 
				UNRESOLVED( -1 , "Child process was killed"); 
			}
			
			if (WIFEXITED(status))
			{
				ret = WEXITSTATUS(status);
			}
			else
			{
				UNRESOLVED( -1, "Child process was neither killed nor exited");
			}
			
			if (ret != 0)
			{
				return ret ; /* Output has already been closed in child */
			}
	
		}
		else /* child was a thread */
		{
			ret = pthread_join(child_th, NULL);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to join the thread");  }
		}
		
		/* Check the child status */
		if (td->status != EBUSY)  
		{
			pthread_mutex_trylock_output("Unexpected return value: %d (%s)\n", td->status, strerror(td->status));
			FAILED("pthread_mutex_trylock() did not return EBUSY in the child");
		}
		
		/* Unlock the mutex */
		ret= pthread_mutex_unlock(&(td->mtx));
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to unlock the mutex");  }
		
/**********
 * Destroy the data 
 */
		ret = pthread_mutex_destroy(&(td->mtx));
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex");  }
		
		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex attribute object");  }
			
	}  /* Proceed to the next scenario */
	
	#if VERBOSE > 0
	pthread_mutex_trylock_output("Test passed\n");
	#endif

	PASSED;
}

