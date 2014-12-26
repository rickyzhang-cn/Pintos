#ifndef SCHEDULE_H
#define SCHEDULE_H

bool check_preemption(void);
bool condvar_more_func(const struct list_elem *a,
		const struct list_elem *b,void *aux);

#endif
