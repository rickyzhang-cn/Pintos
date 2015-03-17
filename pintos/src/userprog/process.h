#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

struct mmap_file {
	struct sup_page_entry *spte;
	int mapid;
	struct list_elem elem;
};

bool process_add_mmap(struct sup_page_entry *spte);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
