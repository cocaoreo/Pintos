#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

struct lock filelock;

void addr_check(void * addr);
void syshalt();
void sysexit();
tid_t sysexec(const char *comdline);
int syswait(tid_t tid);
int sysread(int fd, void *buffer, unsigned size);
int syswrite(int fd, const void *buffer, unsigned size);

bool syscreate(const char * name, int initial_size);
bool sysremove(const char * name);
int sysopen(const char * file);
void sysclose(int fd);
int sysfilesize(int fd);
void sysseek(int fd, unsigned position);
unsigned systell (int fd);

// read, write, seek, tell
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filelock);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  //printf ("system call!\n");
  addr_check(f->esp);

  int sysnum = *(int *)(f->esp);

  switch(sysnum){
    case SYS_HALT:
      syshalt();
      break;
    case SYS_EXIT:
      addr_check(f->esp+4);
      sysexit(*(int *)(f->esp+4));
      break;
    case SYS_EXEC:
      addr_check(f->esp+4);
      f->eax = sysexec((char*)*(int *)(f->esp+4));
      break;
    case SYS_WAIT:
      addr_check(f->esp+4);
      f->eax = syswait(*(int *)(f->esp+4));
      break;
    case SYS_READ:
      addr_check(f->esp+4);
      addr_check(f->esp+8);
      addr_check(f->esp+12);
      f->eax = sysread(*(int *)(f->esp+4), *(int *)(f->esp+8), *(unsigned* )(f->esp+12));
      break;
    case SYS_WRITE:
      addr_check(f->esp+4);
      addr_check(f->esp+8);
      addr_check(f->esp+12);
      f->eax = syswrite(*(int *)(f->esp+4), *(int *)(f->esp+8), *(unsigned* )(f->esp+12));
      break;
    case SYS_FIBO:
      addr_check(f->esp+4);
      f->eax = fibonacci(*(int *)(f->esp+4));
      break;
    case SYS_MAX:
      addr_check(f->esp+4);
      addr_check(f->esp+8);
      addr_check(f->esp+12);
      addr_check(f->esp+16);
      f->eax = max_of_four_int(*(int *)(f->esp+4), *(int *)(f->esp+8), *(int *)(f->esp+12), *(int *)(f->esp+16));
      break;
    case SYS_CREATE:
      addr_check(f->esp+4);
      addr_check(f->esp+8);
      f->eax = syscreate((char*)*(int *)(f->esp+4), *(int *)(f->esp+8));
      break;
    case SYS_REMOVE:
      addr_check(f->esp+4);
      f->eax = sysremove((char*)*(int *)(f->esp+4));
      break;
    case SYS_OPEN:
      addr_check(f->esp+4);
      f->eax = sysopen((char*)*(int *)(f->esp+4));
      break;
    case SYS_CLOSE:
      addr_check(f->esp+4);
      sysclose(*(int *)(f->esp+4));
      break;
    case SYS_FILESIZE:
      addr_check(f->esp+4);
      f-> eax = sysfilesize(*(int *)(f->esp+4));
      break;
    case SYS_SEEK:
      addr_check(f->esp+4);
      addr_check(f->esp+8);
      sysseek(*(int *)(f->esp+4),*(unsigned *)(f->esp+8));
      break;
    case SYS_TELL:
      addr_check(f->esp+4);
      f-> eax = systell(*(unsigned *)(f->esp+4));
      break;
    default:
      break;
  }
  //thread_exit ();

}

void addr_check(void * addr){
  if(addr == NULL||addr<=0){
    sysexit(-1);
  }

  if(!is_user_vaddr(addr)||is_kernel_vaddr(addr)){
    sysexit(-1);
  }

  struct thread *t = thread_current();

  if(pagedir_get_page(t->pagedir, addr)==NULL){
    sysexit(-1);
  }
}

void syshalt(){
  shutdown_power_off();
}

void sysexit(int status){
  struct thread *t = thread_current();
  printf("%s: exit(%d)\n", t->name, status);
  t->exitstatus = status;
  thread_exit();
}

tid_t sysexec(const char* cmdline){
  addr_check((void*)cmdline);
  char temp[128];
  strlcpy (temp, cmdline, sizeof(temp));
  tid_t t = process_execute(temp);

  return t;
}

int syswait(tid_t tid){
  return process_wait(tid);
}

int sysread(int fd, void *buffer, unsigned size){
  if(fd==0){
    return input_getc();
  }
  else if (fd > 2 && fd<128){
    addr_check(buffer);
    //addr_check(buffer+size);
	if((size_t)buffer < PHYS_BASE - 0x800000)
		sysexit(-1);
    lock_acquire(&filelock);
    struct file * f = thread_current()->file[fd];
    if(f == NULL){
      sysexit(-1);
    }
    int t = file_read(f,buffer,size);
    lock_release(&filelock);
    return t;
  }
  return -1;
}

int syswrite(int fd, const void *buffer, unsigned size){
  if(fd==1){
    lock_acquire(&filelock);
    addr_check(buffer);
    //addr_check(buffer+size);
    putbuf(buffer, size);
    lock_release(&filelock);
    return size;
  }

  else if (fd > 2 && fd<128){
    addr_check(buffer);
    //addr_check(buffer+size);
    lock_acquire(&filelock);
    struct file * f = thread_current()->file[fd];
    if(f == NULL){
      sysexit(-1);
    }

    int t = file_write(f,buffer,size);
    lock_release(&filelock);
    return t;
  }

  return -1;
}

int fibonacci(int n){
  int result=0;

  if(n<0){
    result = -1;
  }
  else if(n==0){
    result=0;
  }
  else if(n==1){
    result = 1;
  }
  else if(n==2){
    result = 1;
  }
  else{
    int a=1, b=1;

    for(int i=3;i<=n;i++){
      result = a+b;
      a = b;
      b = result;
    }
  }

  return result;
}

int max_of_four_int(int a, int b, int c, int d){
  if(a<c){
    a = c;
  }

  if(b<d){
    b = d;
  }

  if(a<b){
    a = b;
  }
  return a;
}

bool syscreate(const char * name, int initial_size){
  addr_check((void*)name);
  char temp[15];
  if(strlen(name)>14){
    return 0;
  }
  strlcpy (temp, name, sizeof(temp));
  lock_acquire(&filelock);
  bool t = filesys_create(temp, initial_size);
  lock_release(&filelock);
  
  return t;
}

bool sysremove(const char * name){
  addr_check((void *)name);
  char temp[15];
  if(strlen(name)>14){
    return 0;
  }
  strlcpy (temp, name, sizeof(temp));
  lock_acquire(&filelock);
  bool t = filesys_remove(temp);
  lock_release(&filelock);
  return t;
}

int sysopen(const char * name){
  addr_check((void *)name);
  char temp[15];
  if(strlen(name)>14){
    return -1;
  }
  strlcpy (temp, name, sizeof(temp));
  lock_acquire(&filelock);

  struct file * f = filesys_open(name);

  if(f == NULL){
    lock_release(&filelock);
    return -1;
  }

  else{
    for(int i=3;i<128;i++){
      if(thread_current()->file[i]==NULL){
        thread_current()->file[i] = f;
        lock_release(&filelock);
        return i;
      }
    }
  }
  file_close(f);
  lock_release(&filelock);
  return -1;
}

void sysclose(int fd){
  lock_acquire(&filelock);
  struct file * f = thread_current()->file[fd];
  if(f == NULL){
    lock_release(&filelock);
    sysexit(-1);
  }

  file_close(f);
  thread_current()->file[fd]=NULL;
  lock_release(&filelock);
}

int sysfilesize(int fd){
  if(thread_current()->file[fd]==NULL){
    sysexit(-1);
  }
  return file_length(thread_current()->file[fd]);
}

void sysseek(int fd, unsigned position){
  if(thread_current()->file[fd]==NULL){
    sysexit(-1);
  }
  file_seek(thread_current()->file[fd], position);
}

unsigned systell (int fd){
  if(thread_current()->file[fd]==NULL){
    sysexit(-1);
  }

  return file_tell(thread_current()->file[fd]);
}
