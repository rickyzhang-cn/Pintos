#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "threads/sched.h"
#include "threads/thread.h"

bool check_preemption()
{
	struct thread *cur=thread_current();
	struct list_elem *top=thread_ready_first();
	return less_func(top,&(cur->elem),"p");
}
