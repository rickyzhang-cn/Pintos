#include <string.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "vm/swap.h"

extern struct lock file_lock;

static unsigned page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
	struct sup_page_entry *spte = hash_entry(e, struct sup_page_entry,
			elem);
	return hash_int((int) spte->uva);
}

static bool page_less_func (const struct hash_elem *a,
		const struct hash_elem *b,
		void *aux UNUSED)
{
	struct sup_page_entry *sa = hash_entry(a, struct sup_page_entry, elem);
	struct sup_page_entry *sb = hash_entry(b, struct sup_page_entry, elem);
	if (sa->uva < sb->uva)
	{
		return true;
	}
	return false;
}

static void page_action_func (struct hash_elem *e, void *aux UNUSED)
{
	struct sup_page_entry *spte = hash_entry(e, struct sup_page_entry,
			elem);
	if (spte->is_loaded)
	{
		frame_free(pagedir_get_page(thread_current()->pagedir, spte->uva));
		pagedir_clear_page(thread_current()->pagedir, spte->uva);
	}
	free(spte);
}

void page_table_init (struct hash *spt)
{
	hash_init (spt, page_hash_func, page_less_func, NULL);
}

void page_table_destroy (struct hash *spt)
{
	hash_destroy (spt, page_action_func);
}

struct sup_page_entry* get_spte (void *uva)
{
	struct sup_page_entry spte;
	spte.uva = pg_round_down(uva);

	struct hash_elem *e = hash_find(&thread_current()->spt, &spte.elem);
	if (!e)
	{
		return NULL;
	}
	return hash_entry (e, struct sup_page_entry, elem);
}

bool load_page (struct sup_page_entry *spte)
{
	bool success = false;
	spte->pinned = true;
	if (spte->is_loaded)
	{
		return success;
	}
	switch (spte->type)
	{
		case FILE:
			success = load_file(spte);
			break;
		case SWAP:
			success = load_swap(spte);
			break;
		case MMAP:
			success = load_file(spte);
			break;
	}
	return success;
}

bool load_swap (struct sup_page_entry *spte)
{
	uint8_t *frame = frame_alloc (PAL_USER, spte);
	if (!frame)
	{
		return false;
	}
	if (!install_page(spte->uva, frame, spte->writable))
	{
		frame_free(frame);
		return false;
	}
	swap_in(spte->swap_index, spte->uva);
	spte->is_loaded = true;
	return true;
}

bool load_file (struct sup_page_entry *spte)
{
	enum palloc_flags flags = PAL_USER;
	if (spte->read_bytes == 0)
	{
		flags |= PAL_ZERO;
	}
	uint8_t *frame = frame_alloc(flags, spte);
	if (!frame)
	{
		return false;
	}
	if (spte->read_bytes > 0)
	{
		lock_acquire(&file_lock);
		if ((int) spte->read_bytes != file_read_at(spte->file, frame,
					spte->read_bytes,
					spte->offset))
		{
			lock_release(&file_lock);
			frame_free(frame);
			return false;
		}
		lock_release(&file_lock);
		memset(frame + spte->read_bytes, 0, spte->zero_bytes);
	}

	if (!install_page(spte->uva, frame, spte->writable))
	{
		frame_free(frame);
		return false;
	}

	spte->is_loaded = true;  
	return true;
}

bool add_file_to_page_table (struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes,
		bool writable)
{
	struct sup_page_entry *spte = malloc(sizeof(struct sup_page_entry));
	if (!spte)
	{
		return false;
	}
	spte->file = file;
	spte->offset = ofs;
	spte->uva = upage;
	spte->read_bytes = read_bytes;
	spte->zero_bytes = zero_bytes;
	spte->writable = writable;
	spte->is_loaded = false;
	spte->type = FILE;
	spte->pinned = false;

	return (hash_insert(&thread_current()->spt, &spte->elem) == NULL);
}

bool add_mmap_to_page_table(struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes)
{
	struct sup_page_entry *spte = malloc(sizeof(struct sup_page_entry));
	if (!spte)
	{
		return false;
	}
	spte->file = file;
	spte->offset = ofs;
	spte->uva = upage;
	spte->read_bytes = read_bytes;
	spte->zero_bytes = zero_bytes;
	spte->is_loaded = false;
	spte->type = MMAP;
	spte->writable = true;
	spte->pinned = false;

	if (!process_add_mmap(spte))
	{
		free(spte);
		return false;
	}

	if (hash_insert(&thread_current()->spt, &spte->elem))
	{
		spte->type = HASH_ERROR;
		return false;
	}
	return true;
}

bool grow_stack (void *uva)
{
	if ( (size_t) (PHYS_BASE - pg_round_down(uva)) > MAX_STACK_SIZE)
	{
		return false;
	}
	struct sup_page_entry *spte = malloc(sizeof(struct sup_page_entry));
	if (!spte)
	{
		return false;
	}
	spte->uva = pg_round_down(uva);
	spte->is_loaded = true;
	spte->writable = true;
	spte->type = SWAP;
	spte->pinned = true;

	uint8_t *frame = frame_alloc (PAL_USER, spte);
	if (!frame)
	{
		free(spte);
		return false;
	}

	if (!install_page(spte->uva, frame, spte->writable))
	{
		free(spte);
		frame_free(frame);
		return false;
	}

	if (intr_context())
	{
		spte->pinned = false;
	}

	return (hash_insert(&thread_current()->spt, &spte->elem) == NULL);
}

