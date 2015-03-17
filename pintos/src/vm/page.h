#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "vm/frame.h"

#define FILE 0
#define SWAP 1
#define MMAP 2
#define HASH_ERROR 3

// 256 KB
#define MAX_STACK_SIZE (1 << 23)

struct sup_page_entry {
	uint8_t type;
	void *uva;
	bool writable;

	bool is_loaded;
	bool pinned;

	// For files
	struct file *file;
	size_t offset;
	size_t read_bytes;
	size_t zero_bytes;

	// For swap
	size_t swap_index;

	struct hash_elem elem;
};

void page_table_init (struct hash *spt);
void page_table_destroy (struct hash *spt);

bool load_page (struct sup_page_entry *spte);
bool load_mmap (struct sup_page_entry *spte);
bool load_swap (struct sup_page_entry *spte);
bool load_file (struct sup_page_entry *spte);
bool add_file_to_page_table (struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes,
		bool writable);
bool add_mmap_to_page_table(struct file *file, int32_t ofs, uint8_t *upage,
		uint32_t read_bytes, uint32_t zero_bytes);
bool grow_stack (void *uva);
struct sup_page_entry* get_spte (void *uva);

#endif /* vm/page.h */

