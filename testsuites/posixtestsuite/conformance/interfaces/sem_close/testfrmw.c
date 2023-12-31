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
 *
 
 
 * This file is a wrapper to use the tests from the NPTL Test & Trace Project
 * with either the Linux Test Project or the Open POSIX Test Suite.
 
 * The following function are defined:
 * void sem_close_output_init()
 * void sem_close_output_fini()
 * void sem_close_output(char * string, ...)
 * 
 * The are used to sem_close_output informative text (as a printf).
 */

#include <time.h>
#include <sys/types.h>
 
/* We use a mutex to avoid conflicts in traces */
static pthread_mutex_t sem_close_m_trace = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************************/
/******************************* stdout module *****************************************/
/*****************************************************************************************/
/* The following functions will sem_close_output to stdout */
#if (1)
static void sem_close_output_init()
{
	/* do nothing */
	return;
}
static void sem_close_output( char * string, ... )
{
   va_list ap;
   char *ts="[??:??:??]";
   struct tm * now;
   time_t nw;

   pthread_mutex_lock(&sem_close_m_trace);
   nw = time(NULL);
   now = localtime(&nw);
   if (now == NULL)
      printf(ts);
   else
      printf("[%2.2d:%2.2d:%2.2d]", now->tm_hour, now->tm_min, now->tm_sec);
   va_start( ap, string);
   vprintf(string, ap);
   va_end(ap);
   pthread_mutex_unlock(&sem_close_m_trace);
}
static void sem_close_output_fini()
{
	/*do nothing */
	return;
}
#endif

