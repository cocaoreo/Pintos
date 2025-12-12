#ifndef VM_FRAME
#define VM_FRAME

#include <hash.h>
#include <list.h>
#include "threads/thread.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "vm/page.h"

void lru_list_init(void);
void add_frame_to_lru_list(struct frame *frame);
void del_frame_from_lru_list(struct frame *frame);

struct frame *alloc_kframe(enum palloc_flags flags);
void free_frame(void *kaddr);
static struct list_elem *get_next_lru_clock(void);
#endif