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

 
 * This file tests the following assertion:
 * 
 * The pthread_cond_broadcast function unblocks all the threads blocked on the
 * conditional variable.
 
 * The steps are:
 *  -> Create N threads which will wait on a condition variable
 *  -> broadcast the condition
 *  -> Every child checks that it owns the mutex (when possible)
 *
 */
 
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We need the XSI extention for the mutex attributes */
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
 #include <signal.h>
 #include <string.h>
 #include <time.h>
 #include <sys/mman.h>
 #include <sys/wait.h>
 
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
  * void output_init()
  * void output(char * string, ...)
  * 
  * Those may be used to output information.
  */
#define UNRESOLVED_KILLALL(error, text) { \
	if (td->fork) \
	{ \
		int _nch; \
		for (_nch=0; _nch<children.nb; _nch++) \
			kill(children.ch[_nch].p, SIGKILL); \
	} \
	UNRESOLVED(error, text); \
	}
#define FAILED_KILLALL(text, Tchild) { \
	if (td->fork) \
	{ \
		int _nch; \
		for (_nch=0; _nch<children.nb; _nch++) \
			kill(children.ch[_nch].p, SIGKILL); \
	} \
	FAILED(text); \
	}
/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define NCHILDREN (20)

#define TIMEOUT  (120)


#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK  /* make tests with MONOTONIC CLOCK if supported */
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

#ifdef WITHOUT_XOPEN
/* We define those to avoid compilation errors, but they won't be used */
#define PTHREAD_MUTEX_DEFAULT 0
#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_ERRORCHECK 0
#define PTHREAD_MUTEX_RECURSIVE 0

#endif


struct _scenar
{
	int m_type; /* Mutex type to use */
	int mc_pshared; /* 0: mutex and cond are process-private (default) ~ !0: Both are process-shared, if supported */
	int c_clock; /* 0: cond uses the default clock. ~ !0: Cond uses monotonic clock, if supported. */
	int fork; /* 0: Test between threads. ~ !0: Test across processes, if supported (mmap) */
	char * descr; /* Case description */
}
scenarii[] =
{
	 {PTHREAD_MUTEX_DEFAULT,    0, 0, 0, "Default mutex"}
	,{PTHREAD_MUTEX_NORMAL,     0, 0, 0, "Normal mutex"}
	,{PTHREAD_MUTEX_ERRORCHECK, 0, 0, 0, "Errorcheck mutex"}
	,{PTHREAD_MUTEX_RECURSIVE,  0, 0, 0, "Recursive mutex"}

	,{PTHREAD_MUTEX_DEFAULT,    1, 0, 0, "PShared default mutex"}
	,{PTHREAD_MUTEX_NORMAL,     1, 0, 0, "Pshared normal mutex"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, 0, 0, "Pshared errorcheck mutex"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, 0, 0, "Pshared recursive mutex"}

	,{PTHREAD_MUTEX_DEFAULT,    1, 0, 1, "Pshared default mutex across processes"}
	,{PTHREAD_MUTEX_NORMAL,     1, 0, 1, "Pshared normal mutex across processes"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, 0, 1, "Pshared errorcheck mutex across processes"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, 0, 1, "Pshared recursive mutex across processes"}

#ifdef USE_ALTCLK
	,{PTHREAD_MUTEX_DEFAULT,    1, 1, 1, "Pshared default mutex and alt clock condvar across processes"}
	,{PTHREAD_MUTEX_NORMAL,     1, 1, 1, "Pshared normal mutex and alt clock condvar across processes"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, 1, 1, "Pshared errorcheck mutex and alt clock condvar across processes"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, 1, 1, "Pshared recursive mutex and alt clock condvar across processes"}

	,{PTHREAD_MUTEX_DEFAULT,    0, 1, 0, "Default mutex and alt clock condvar"}
	,{PTHREAD_MUTEX_NORMAL,     0, 1, 0, "Normal mutex and alt clock condvar"}
	,{PTHREAD_MUTEX_ERRORCHECK, 0, 1, 0, "Errorcheck mutex and alt clock condvar"}
	,{PTHREAD_MUTEX_RECURSIVE,  0, 1, 0, "Recursive mutex and alt clock condvar"}

	,{PTHREAD_MUTEX_DEFAULT,    1, 1, 0, "PShared default mutex and alt clock condvar"}
	,{PTHREAD_MUTEX_NORMAL,     1, 1, 0, "Pshared normal mutex and alt clock condvar"}
	,{PTHREAD_MUTEX_ERRORCHECK, 1, 1, 0, "Pshared errorcheck mutex and alt clock condvar"}
	,{PTHREAD_MUTEX_RECURSIVE,  1, 1, 0, "Pshared recursive mutex and alt clock condvar"}
#endif
};
#define NSCENAR (sizeof(scenarii)/sizeof(scenarii[0]))

/* The shared data */
typedef struct 
{
	int 		count;     /* number of children currently waiting */
	pthread_cond_t 	cnd;
	pthread_mutex_t mtx;
	int 		predicate; /* Boolean associated to the condvar */
	clockid_t 	cid;       /* clock used in the condvar */
	int		mtype;     /* Type of the mutex */
	char		fork;      /* the children are processes */
} testdata_t;
testdata_t * td;

struct 
{
	union
	{
		pthread_t t;
		pid_t p;
	} ch[NCHILDREN];
	int nb;
} children;


/* Child function (either in a thread or in a process) */
void * pthread_cond_broadcast_2_3_child(void * arg)
{
	int ret=0;
	int timed;
	struct timespec ts;
	
	/* lock the mutex */
	ret = pthread_mutex_lock(&td->mtx);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to lock mutex in child");  }
	
	/* increment count */
	td->count++;
	timed=td->count & 1;
	
	if (timed)
	{
	/* get current time if we are a timedwait */
		ret = clock_gettime(td->cid, &ts);
		if (ret != 0)  {  UNRESOLVED(errno, "Unable to read clock");  }
		ts.tv_sec += TIMEOUT;
	}
	
	do {
	/* Wait while the predicate is false */
		if (timed)
			ret = pthread_cond_timedwait(&td->cnd, &td->mtx, &ts);
		else
			ret = pthread_cond_wait(&td->cnd, &td->mtx);
		#if VERBOSE > 5
		output("[child] Wokenup timed=%i, Predicate=%i, ret=%i\n", timed, td->predicate, ret);
		#endif
	} while ((ret == 0) && (td->predicate==0));
	if (ret == ETIMEDOUT)
	{
		FAILED("Timeout occured. This means a cond signal was lost -- or parent died");
	}
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to wait for the cond");  }
	
	/* Check that we are owning the mutex */
	#ifdef WITHOUT_XOPEN
	ret = pthread_mutex_trylock(&(td->mtx));
	if (ret == 0)  {  FAILED("The mutex was not owned after return from condition waiting");  }
	#else
	if (td->mtype == PTHREAD_MUTEX_RECURSIVE)
	{
		ret = pthread_mutex_trylock(&(td->mtx));
		if (ret != 0)  {  FAILED("Unable to relock recursive mutex: not owning?");  }
		ret = pthread_mutex_unlock(&(td->mtx));
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to unlock the mutex");  }
	}
	if (td->mtype == PTHREAD_MUTEX_ERRORCHECK)
	{
		ret = pthread_mutex_lock(&(td->mtx));
		if (ret == 0)  {  FAILED("Was able to lock errorcheck mutex: the mutex was not acquired once already?");  }
	}
	#endif
	
	
	/* unlock the mutex */
	ret = pthread_mutex_unlock(&td->mtx);
	if (ret != 0)  {  UNRESOLVED(ret, "Failed to unlock the mutex.");  }
	
	return NULL;
}

/* Timeout thread */
void * pthread_cond_broadcast_2_3_timer(void * arg)
{
	unsigned int to = TIMEOUT;
	do { to = sleep(to); }
	while (to>0);
	FAILED_KILLALL("Operation timed out. A signal was lost.", pchildren); 
	return NULL; /* For compiler */
}

/* main function */

int pthread_cond_broadcast_2_3(int argc, char * argv[])
{
	int ret;
	
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	
	int scenar;
	long pshared, monotonic, cs, mf;
	
	int child_count;
	
	pid_t pid;
	int status;
	
	pthread_t t_timer;
	
	pthread_attr_t ta;
	
	testdata_t alternativ;
	
	output_init();
	
	/* check the system abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	cs = sysconf(_SC_CLOCK_SELECTION);
	monotonic = sysconf(_SC_MONOTONIC_CLOCK);
	mf =sysconf(_SC_MAPPED_FILES);
	
	#if VERBOSE > 0
	output("Test starting\n");
	output("System abilities:\n");
	output(" TPS : %li\n", pshared);
	output(" CS  : %li\n", cs);
	output(" MON : %li\n", monotonic);
	output(" MF  : %li\n", mf);
	if ((mf < 0) || (pshared < 0))
		output("Process-shared attributes won't be tested\n");
	if ((cs < 0) || (monotonic < 0))
		output("Alternative clock won't be tested\n");
	#endif

	/* We are not interested in testing the clock if we have no other clock available.. */
	if (monotonic < 0)
		cs = -1;
	
#ifndef USE_ALTCLK
	if (cs > 0)
		output("Implementation supports the MONOTONIC CLOCK but option is disabled in test.\n");
#endif
	
/**********
 * Allocate space for the testdata structure
 */
	if (mf < 0)
	{
		/* Cannot mmap a file, we use an alternative method */
		td = &alternativ;
		pshared = -1; /* We won't do this testing anyway */
		#if VERBOSE > 0
		output("Testdata allocated in the process memory.\n");
		#endif
	}
	else
	{
		/* We will place the test data in a mmaped file */
		char filename[] = "/tmp/cond_broadcast-XXXXXX";
		size_t sz, ps;
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
		output("Temp file created (%s).\n", filename);
		#endif
		
		ps = (size_t)sysconf(_SC_PAGESIZE);
		sz= ((sizeof(testdata_t) / ps) + 1) * ps; /* # pages needed to store the testdata */
		
		tmp = calloc( 1 , sz);
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
		
		td = (testdata_t *) mmaped;
		
		/* Our datatest structure is now in shared memory */
		#if VERBOSE > 1
		output("Testdata allocated in shared memory (%ib).\n", sizeof(testdata_t));
		#endif
	}
	
	/* Initialize the thread attribute object */
	ret = pthread_attr_init(&ta);
	if (ret != 0)  {  UNRESOLVED(ret, "[parent] Failed to initialize a thread attribute object");  }
	ret = pthread_attr_setstacksize(&ta, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0)  {  UNRESOLVED(ret, "[parent] Failed to set thread stack size");  }
	
	/* Do the test for each test scenario */
	for (scenar=0; scenar < NSCENAR; scenar++)
	{
		/* set / reset everything */
		td->fork=0;
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to initialize the mutex attribute object");  }
		ret = pthread_condattr_init(&ca);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to initialize the cond attribute object");  }
		
		#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, scenarii[scenar].m_type);
		if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set mutex type");  }
		#endif
		
		td->mtype=scenarii[scenar].m_type;
		
		
		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[scenar].mc_pshared != 0))
		{
			ret = pthread_mutexattr_setpshared(&ma, PTHREAD_PROCESS_SHARED);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set the mutex process-shared");  }
			ret = pthread_condattr_setpshared(&ca, PTHREAD_PROCESS_SHARED);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set the cond var process-shared");  }
		}
		
		/* Set the alternative clock, if supported */
		#ifdef USE_ALTCLK
		if ((cs > 0) && (scenarii[scenar].c_clock != 0))
		{
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0)  {  UNRESOLVED(ret, "[parent] Unable to set the monotonic clock for the cond");  }
		}
		ret = pthread_condattr_getclock(&ca, &td->cid);
		if (ret != 0)  {  UNRESOLVED(ret, "Unable to get clock from cond attr");  }
		#else
		td->cid = CLOCK_REALTIME;
		#endif
		
		/* Tell whether the test will be across processes */
		if ((pshared > 0) && (scenarii[scenar].fork != 0))
		{
			td->fork = 1;
		}

		/* initialize the condvar */
		ret = pthread_cond_init(&td->cnd, &ca);
		if (ret != 0)  {  UNRESOLVED(ret, "Cond init failed");  }
		
		/* initialize the mutex */
		ret = pthread_mutex_init(&td->mtx, &ma);
		if (ret != 0)  {  UNRESOLVED(ret, "Mutex init failed");  }
		
		/* Destroy the attributes */
		ret = pthread_condattr_destroy(&ca);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the cond var attribute object");  }
		
		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex attribute object");  }
		
		#if VERBOSE > 2
		output("[parent] Starting test %s\n", scenarii[scenar].descr);
		#endif
		
		td->count=0;
		
		/* Create all the children */
		for (children.nb=0; children.nb<NCHILDREN; children.nb++)
		{
			if (td->fork==0)
			{
				ret = pthread_create(&(children.ch[children.nb].t), &ta, pthread_cond_broadcast_2_3_child, NULL);
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to create enough threads");  }
			}
			else
			{
				children.ch[children.nb].p=fork();
				if (children.ch[children.nb].p == 0) /* We are the child */
				{
                    pthread_cond_broadcast_2_3_child(NULL);
					exit(0);
				}
				if (children.ch[children.nb].p == -1) /* The fork failed */
				{
					children.nb--;
					UNRESOLVED_KILLALL(errno, "Failed to create enough processes");
				}
			}
		}
		#if VERBOSE > 4
		output("[parent] Created %i children\n", NCHILDREN);
		#endif
		
		/* Make sure all children are waiting */
		ret = pthread_mutex_lock(&td->mtx);
		if (ret != 0) {  UNRESOLVED_KILLALL(ret, "Failed to lock mutex");  }
		child_count = td->count;
		while (child_count < NCHILDREN)
		{
			ret = pthread_mutex_unlock(&td->mtx);
			if (ret != 0)  {  UNRESOLVED_KILLALL(ret, "Failed to unlock mutex");  }
			sched_yield();
			ret = pthread_mutex_lock(&td->mtx);
			if (ret != 0) {  UNRESOLVED_KILLALL(ret, "Failed to lock mutex");  }
			child_count = td->count;
		}
		
		#if VERBOSE > 4
		output("[parent] All children are waiting\n");
		#endif
		
		/* create the timeout thread */
		ret = pthread_create(&t_timer, NULL, pthread_cond_broadcast_2_3_timer, NULL);
		if (ret != 0)  {  UNRESOLVED_KILLALL(ret, "Unable to create timer thread");  }
		
		/* Wakeup the children */
		td->predicate=1;
		ret = pthread_cond_broadcast(&td->cnd);
		if (ret != 0)  {  UNRESOLVED_KILLALL(ret, "Failed to broadcast the condition.");  }

		#if VERBOSE > 4
		output("[parent] Condition was signaled\n");
		#endif
		
		ret = pthread_mutex_unlock(&td->mtx);
		if (ret != 0)  {  UNRESOLVED_KILLALL(ret, "Failed to unlock mutex");  }
		
		#if VERBOSE > 4
		output("[parent] Joining the children\n");
		#endif

		/* join the children */
		for (--children.nb; children.nb >= 0; children.nb--)
		{
			if (td->fork==0)
			{
				ret = pthread_join(children.ch[children.nb].t, NULL);
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to join a child thread");  }
			}
			else
			{
				pid = waitpid(children.ch[children.nb].p, &status, 0);
				if (pid != children.ch[children.nb].p)
				{
					ret = errno;
					output("Waitpid failed (expected: %i, got: %i)\n", children.ch[children.nb].p, pid);
					UNRESOLVED_KILLALL(ret, "Waitpid failed");
				}
				if (WIFEXITED(status))
				{
					/* the child should return only failed or unresolved or passed */
					if (ret != PTS_FAIL)
						ret |= WEXITSTATUS(status);
				}
			}
		}
		if (ret != 0)
		{
			output_fini();
			exit(ret);
		}
		#if VERBOSE > 4
		output("[parent] All children terminated\n");
		#endif
		
		
		/* cancel the timeout thread */
		ret = pthread_cancel(t_timer);
		if (ret != 0)
		{
			/* Strange error here... the thread cannot be terminated (app would be killed) */
			UNRESOLVED(ret, "Failed to cancel the timeout handler");
		}
		
		/* join the timeout thread */
		ret = pthread_join(t_timer, NULL);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to join the timeout handler");  }
			
		/* Destroy the datas */
		ret = pthread_cond_destroy(&td->cnd);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the condvar");  }
		
		ret = pthread_mutex_destroy(&td->mtx);
		if (ret != 0)  {  UNRESOLVED(ret, "Failed to destroy the mutex");  }
		
	}
	
	/* Destroy global data */
	ret = pthread_attr_destroy(&ta);
	if (ret != 0)  {  UNRESOLVED(ret, "Final thread attr destroy failed");  }
	
	
	/* exit */
	PASSED;
}

