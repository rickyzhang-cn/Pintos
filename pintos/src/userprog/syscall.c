#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/miscellaneous.h"
#include "filesys/file.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

struct lock file_lock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{
  //printf ("system call!\n");
  //thread_exit ();
  uint32_t *sp=f->esp;

  if(!sp || !is_user_vaddr(sp) || !get_user(sp))
	  sys_exit(-1);

  int syscall_num=*(sp);

  if(syscall_num<SYS_HALT || syscall_num>SYS_INUMBER)
	  sys_exit(-1);

  switch(syscall_num)
  {
  	case SYS_HALT:
	{
		shutdown_power_off();
		break;
	}
	case SYS_EXIT:
	{
  		if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		struct thread *cur=thread_current();
		int status=*(int *)(sp+1);
		cur->exit_status=status;
		thread_exit();
		break;
	}
	case SYS_EXEC:
	{
  		if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		char *cmd_line=*(sp+1);
		if(!cmd_line && !valid(cmd_line))
			sys_exit(-1);
		else
			f->eax=process_execute(cmd_line);
		break;
	}
	case SYS_WAIT:
	{
  		if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		tid_t tid=(tid_t)*(sp+1);
		f->eax=process_wait(tid);
		break;
	}
	case SYS_CREATE:
	{
		sp=sp+3;
  		if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		if(!is_user_vaddr(sp+2))
			sys_exit(-1);
		char *file_name=*(sp+1);
		off_t initial_size=(off_t)*(sp+2);
		if(file_name==NULL || !valid(file_name))
			sys_exit(-1);
		f->eax=filesys_create(file_name,initial_size);
		break;
	}
	case SYS_REMOVE:
	{
  		if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		char *file_name=(char *)(sp+1);
		if(!file_name)
		{
			f->eax=-1;
			break;
		}
		if(!valid(file_name))
		{
			sys_exit(-1);
		}else
		{
			lock_acquire(&file_lock);
			f->eax=filesys_remove(file_name);
			lock_release(&file_lock);
		}
		break;
	}
	case SYS_OPEN:
	{
  		if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		char* file_name=*(sp+1);
		struct file *file;
		if(!file_name)
		{
			f->eax=-1;
			break;
		}
		else if(!valid(file_name))
		{
			sys_exit(-1);
		}
		else
			file=filesys_open(file_name);
		if(!file)
		{
			f->eax=-1;
			break;
		}else
		{
			struct thread *cur=thread_current();
			file->fd=(cur->fd_cnt)++;
			list_push_back(&cur->open_file_list,&file->open_file_elem);
			f->eax=file->fd;
		}
		break;
	}
	case SYS_FILESIZE:
	{
  	  if(!is_user_vaddr(sp+1))
	  	sys_exit(-1);
	  int fd=*(sp+1);
	  struct file *file=get_file(fd);

	  if(!file)
	  {
	  	f->eax=-1;
		break;
	  }else
		  f->eax=file_length(file);
	  break;
	}
	case SYS_READ:
	{
		sp=sp+4;
  	    if(!is_user_vaddr(sp+1))
	  	    sys_exit(-1);
	
		if(!(is_user_vaddr(sp+2) && is_user_vaddr(sp+3)))
			sys_exit(-1);
		
		int fd=*(sp+1);
		uint8_t *u_buffer=*(sp+2);
		unsigned size=*(sp+3);

		if(!u_buffer || !valid(u_buffer))
			sys_exit(-1);
		if(size<0)
			sys_exit(-1);
		if(fd==STDIN_FILENO)
		{
			int i;
			for(i=0;i!=size;i++)
				*(uint8_t *)(u_buffer+i)=input_getc();
			f->eax=size;
		}
		else if(fd==STDOUT_FILENO)
		{
			f->eax=-1;
			break;
		}
		else
		{
			struct file *file=get_file(fd);
			if(!file)
			{
				f->eax=-1;
				break;
			}
			lock_acquire(&file_lock);
			f->eax=file_read(file,u_buffer,size);
			lock_release(&file_lock);
		}
		break;
	}
	case SYS_WRITE:
	{
	  sp=sp+4;//opps!! fix for the wrong stack
  	  if(!is_user_vaddr(sp+1))
	  	  sys_exit(-1);
	  if(!(is_user_vaddr(sp+2) && is_user_vaddr(sp+3)))
		  sys_exit(-1);

	  int fd=*(sp+1);
	  char *buffer=*(sp+2);
	  uint32_t size=*(sp+3);

	  if(!buffer || !valid(buffer))
		  sys_exit(-1);
	  if(size<0)
		  sys_exit(-1);

	  if(fd==STDOUT_FILENO)
		  putbuf(buffer,size);
	  else if(fd==STDIN_FILENO)
	  {
	  	f->eax=-1; //error
	  }
	  else
	  {
		struct file *file=get_file(fd);
		if(!file)
		{
			f->eax=-1;
			break;
		}else
		{
			lock_acquire(&file_lock);
			f->eax=file_write(file,buffer,size);
			lock_release(&file_lock);
		}
	  }
		break;
	}
	case SYS_SEEK:
	{
  	    sp=sp+3;
		if(!is_user_vaddr(sp+1))
	  	    sys_exit(-1);
		if(!is_user_vaddr(sp+2))
			sys_exit(-1);
		int fd=*(sp+1);
		unsigned int pos=*(sp+2);
		struct file *file=get_file(fd);

		if(!file)
		{
			f->eax=-1;
			break;
		}else
		{
			file_seek(file,pos);
			f->eax=0;
		}
		break;
	}
	case SYS_TELL:
	{
  	  	if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		int fd=*(sp+1);

		struct file *file=get_file(fd);

		if(!file)
		{
			f->eax=-1;
			break;
		}else
			f->eax=file_tell(file);
		break;
	
	}
	case SYS_CLOSE:
	{
  	  	if(!is_user_vaddr(sp+1))
	  		sys_exit(-1);
		int fd=(int)*(sp+1);
		struct file *file;
		file=get_file(fd);
		if(!file)
		{
			f->eax=-1;
			sys_exit(-1);
		}else
		{
			struct list_elem *e=&(file->open_file_elem);
			list_remove(e);
			file_close(file);
			f->eax=0;
		}
		break;
	}
  }
}
