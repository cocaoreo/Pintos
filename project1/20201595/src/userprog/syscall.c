#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"

static void syscall_handler (struct intr_frame *);

void addr_check(void * addr);
void syshalt();
void sysexit();
tid_t sysexec(const char *comdline);
int syswait(tid_t tid);
int sysread(int fd, void *buffer, unsigned size);
int syswrite(int fd, const void *buffer, unsigned size);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
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
  return -1;
}

int syswrite(int fd, const void *buffer, unsigned size){
  if(fd==1){
    putbuf(buffer, size);
    return size;
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