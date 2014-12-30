#ifndef SCHEDULE_H
#define SCHEDULE_H
#include "thread/thread.h"
#include "lib/kernel/list.h"

bool check_preemption(void);
bool condvar_more_func(const struct list_elem *a,
		const struct list_elem *b,void *aux);
void thread_set_ready_priority(struct thread *t,int new_priority);
void thread_set_true_priority(struct thread *t);
#endif
