#ifndef VM_SWAP
#define VM_SWAP

#include <hash.h>
#include <list.h>
#include "threads/thread.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "vm/page.h"
#include "vm/swap.h"

void swap_init(size_t idx, void *kaddr);
void swap_in(size_t idx, void *kaddr);
size_t swap_out(void *kaddr);

#endif