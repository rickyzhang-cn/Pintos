#include "vm/swap.h"

void swap_init (void)
{
	swap_block = block_get_role (BLOCK_SWAP);
	if (!swap_block)
	{
		return;
	}
	swap_map = bitmap_create( block_size(swap_block) / SECTORS_PER_PAGE );
	if (!swap_map)
	{
		return;
	}
	bitmap_set_all(swap_map, SWAP_FREE);
	lock_init(&swap_lock);
}


size_t swap_out (void *frame)
{
	if (!swap_block || !swap_map)
	{
		PANIC("Need swap partition but no swap partition present!");
	}
	lock_acquire(&swap_lock);
	size_t free_index = bitmap_scan_and_flip(swap_map, 0, 1, SWAP_FREE);

	if (free_index == BITMAP_ERROR)
	{
		PANIC("Swap partition is full!");
	}

	size_t i;
	for (i = 0; i < SECTORS_PER_PAGE; i++)
	{ 
		block_write(swap_block, free_index * SECTORS_PER_PAGE + i,
				(uint8_t *) frame + i * BLOCK_SECTOR_SIZE);
	}
	lock_release(&swap_lock);
	return free_index;
}

void swap_in (size_t used_index, void* frame)
{
	if (!swap_block || !swap_map)
	{
		return;
	}
	lock_acquire(&swap_lock);
	if (bitmap_test(swap_map, used_index) == SWAP_FREE)
	{
		PANIC ("Trying to swap in a free block! Kernel panicking.");
	}
	bitmap_flip(swap_map, used_index);

	size_t i;
	for (i = 0; i < SECTORS_PER_PAGE; i++)
	{
		block_read(swap_block, used_index * SECTORS_PER_PAGE + i,
				(uint8_t *) frame + i * BLOCK_SECTOR_SIZE);
	}
	lock_release(&swap_lock);
}

