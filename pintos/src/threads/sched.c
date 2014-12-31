#include "threads/synch.h"
#include "lib/kernel/list.h"
#include "threads/sched.h"
#include "threads/thread.h"

bool check_preemption()
{
	struct thread *cur=thread_current();
	struct list_elem *top=thread_ready_first();
	//ASSERT(top->next != NULL);//top can not be the tail;
	if(top->next == NULL) //if the top is the tail,just retrun 0,only exists in init.c
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

void thread_set_ready_priority(struct thread *t,int new_priority)
{
	ASSERT(new_priority >= PRI_MIN && new_priority <= PRI_MAX);
	if(new_priority > (t->priority))
	{
		t->under_donation=true;
		t->priority=new_priority;

		//list_sort(&ready_list,&less_func,"p");
		ready_list_reorder();
	}
}

void thread_set_true_priority(struct thread *t)
{
	t->under_donation=false;

	struct list_elem *e;
	struct list_elem *te;
	struct thread *top_thread;
	struct lock *l;
	int max_priority=t->actual_priority;

	if(list_size(&(t->locks_list)) > 0)
	{
		for(e=list_begin(&(t->locks_list));e!=list_end(&(t->locks_list));e=list_next(e))
		{
			l=list_entry(e,struct lock,locks_elem);
			if(list_empty(&((l->semaphore).waiters)))
				continue;
			else
			{
				te=list_begin(&((l->semaphore).waiters));
				top_thread=list_entry(te,struct thread,elem);
				if(top_thread->priority > max_priority)
					max_priority=top_thread->priority;
			}
		}
	}
	t->priority=max_priority;
	//msg("in thread_set_true_priority,t,name=%s priority=%d\n",t->name,t->priority);
	//list_sort(&ready_list,less_func,"p");
	ready_list_reorder();
}
