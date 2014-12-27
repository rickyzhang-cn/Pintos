#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "threads/sched.h"
#include "threads/thread.h"

bool check_preemption()
{
	struct thread *cur=thread_current();
	struct list_elem *top=thread_ready_first();
	//ASSERT(top->next != NULL);//top can not be the tail;
	if(top->next == NULL)
		return 0;
	return less_func(top,&cur->elem,"p");
}

bool condvar_more_func(const struct list_elem *a,
		const struct list_elem *b,void *aux)
{
	struct semaphore_elem *as=list_entry(a,struct semaphore_elem,elem);
	struct semaphore_elem *bs=list_entry(b,struct semaphore_elem,elem);
	return as->priority > bs->priority;
}
