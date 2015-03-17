#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"

extern struct lock file_lock;

void frame_table_init (void)
{
	list_init(&frame_table);
	lock_init(&frame_table_lock);
}

void* frame_alloc (enum palloc_flags flags, struct sup_page_entry *spte)
{
	if ( (flags & PAL_USER) == 0 )
	{
		return NULL;
	}
	void *frame = palloc_get_page(flags);
	if (frame)
	{
		frame_add_to_table(frame, spte);
	}
	else
	{
		while (!frame)
		{
			frame = frame_evict(flags);
			lock_release(&frame_table_lock);
		}
		if (!frame)
		{
			PANIC ("Frame could not be evicted because swap is full!");
		}
		frame_add_to_table(frame, spte);
	}
	return frame;
}

void frame_free (void *frame)
{
	struct list_elem *e;

	lock_acquire(&frame_table_lock);
	for (e = list_begin(&frame_table); e != list_end(&frame_table);
			e = list_next(e))
	{
		struct frame_entry *fte = list_entry(e, struct frame_entry, elem);
		if (fte->frame == frame)
		{
			list_remove(e);
			free(fte);
			palloc_free_page(frame);
			break;
		}
	}
	lock_release(&frame_table_lock);
}

void frame_add_to_table (void *frame, struct sup_page_entry *spte)
{
	struct frame_entry *fte = malloc(sizeof(struct frame_entry));
	fte->frame = frame;
	fte->spte = spte;
	fte->thread = thread_current();
	lock_acquire(&frame_table_lock);
	list_push_back(&frame_table, &fte->elem);
	lock_release(&frame_table_lock);
}

void* frame_evict (enum palloc_flags flags)
{
	lock_acquire(&frame_table_lock);
	struct list_elem *e = list_begin(&frame_table);

	while (true)
	{
		struct frame_entry *fte = list_entry(e, struct frame_entry, elem);
		if (!fte->spte->pinned)
		{
			struct thread *t = fte->thread;
			if (pagedir_is_accessed(t->pagedir, fte->spte->uva))
			{
				pagedir_set_accessed(t->pagedir, fte->spte->uva, false);
			}
			else
			{
				if (pagedir_is_dirty(t->pagedir, fte->spte->uva) ||
						fte->spte->type == SWAP)
				{
					if (fte->spte->type == MMAP)
					{
						lock_acquire(&file_lock);
						file_write_at(fte->spte->file, fte->frame,
								fte->spte->read_bytes,
								fte->spte->offset);
						lock_release(&file_lock);
					}
					else
					{
						fte->spte->type = SWAP;
						fte->spte->swap_index = swap_out(fte->frame);
					}
				}
				fte->spte->is_loaded = false;
				list_remove(&fte->elem);
				pagedir_clear_page(t->pagedir, fte->spte->uva);
				palloc_free_page(fte->frame);
				free(fte);
				return palloc_get_page(flags);
			}
		}
		e = list_next(e);
		if (e == list_end(&frame_table))
		{
			e = list_begin(&frame_table);
		}
	}
}

