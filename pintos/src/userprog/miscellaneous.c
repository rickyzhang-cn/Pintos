#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/args_passing.h"
#include "userprog/miscellaneous.h"
#include "lib/kernel/list.h"

int get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

bool put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "r" (byte));
  return error_code != -1;
}

void sys_exit(int status){
    struct thread* cur=thread_current();
    cur->exit_status=status;
    thread_exit();
}

bool valid (char *uaddr)
{
  int ch;
  int i=0;
  while ((ch = get_user (uaddr + i++)) != '\0')
    {
      if (ch == -1)
         return false;
    }
  return true;
}

struct file* get_file(int fd)
{
	struct thread *cur=thread_current();
	struct list_elem *e;
	for(e=list_begin(&cur->open_file_list);e!=list_end(&cur->open_file_list);e=list_next(e))
	{
		struct file *file=list_entry(e,struct file,open_file_elem);
		if(file->fd=fd)
			return file;
	}
	return NULL;
}

struct thread* tid_to_thread(tid_t tid)
{
	struct thread *cur=thread_current();
	struct list_elem *e;
	for(e=list_begin(&cur->child_list);e!=list_end(&cur->child_list);e=list_next(e))
	{
		struct thread *t=list_entry(e,struct thread,child_elem);
		if(t->tid == tid)
			return t;
	}
	return NULL;
}
