/* Tests timer_sleep(0), which should return immediately. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

static thread_func rz_thread;

static int i;

#if 0
void
test_alarm_zero (void) 
{
  timer_sleep (0);
  pass ();
}
#endif

void test_alarm_zero(void)
{
	char name[16];
	for(i=0;i<5;i++)
	{
		snprintf(name,sizeof(name),"thread %d",i);
		thread_create(name,PRI_DEFAULT,rz_thread,&i);
	}
}

static void rz_thread(void *aux)
{
	int i=*((int *)aux);
	while(1)
		printf("%s\n",thread_current()->name);
}
