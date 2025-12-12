#include "vm/page.h"
#include "vm/frame.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "filesys/file.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <list.h>

struct list lru_list;
struct lock lru_lock;
struct list_elem *lruclock;

void lru_list_init()
{
  list_init(&lru_list);
  lock_init(&lru_lock);
  lruclock = NULL;
}

void add_frame_to_lru_list(struct frame *frame)
{
  if (frame)
  {
    lock_acquire(&lru_lock);
    list_push_back(&lru_list, &frame->lru);
    lock_release(&lru_lock);
  }
}

void del_frame_to_lru_list(struct frame *frame)
{
  if (frame)
  {
    lock_acquire(&lru_lock);
    list_remove(&frame->lru);
    lock_release(&lru_lock);
  }
}

struct frame *alloc_kframe(enum palloc_flags flags)
{
  uint8_t *kpage = palloc_get_page(flags);
  struct frame *newfr = malloc(sizeof(struct frame));

  newfr->kaddr = kpage;
  newfr->thread = thread_current();
  add_frame_to_lru_list(&(newfr->lru));
  return newfr;
}

void free_frame(void *kaddr)
{
  struct list_elem *start = list_begin(&lru_list);
  struct list_elem *end = list_end(&lru_list);

  lock_acquire(&lru_lock);
  for (struct list_elem *t = start; t != end; t = list_next(t))
  {
    struct frame *temp = list_entry(t, struct frame, lru);
    if (temp->kaddr == kaddr)
    {
      del_frame_to_lru_list(&temp->lru);
      palloc_free_page(temp->kaddr);
      free(temp);
    }
  }
  lock_release(&lru_lock);
}

static struct list_elem *get_next_lru_clock()
{
  struct list_elem *start = lruclock;
  struct list_elem *end = list_end(&lru_list);

  if (lruclock == NULL)
  {
    lruclock = list_begin(&lru_list);
  }

  else if (lruclock == end)
  {
    lruclock = list_begin(&lru_list);
  }
  else
  {
    lruclock = list_next(lruclock);
    if (lruclock == end)
    {
      lruclock = list_begin(&lru_list);
    }
  }

  return lruclock;
}