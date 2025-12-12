#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "filesys/file.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void vm_init(struct hash *vm)
{
  hash_init(vm, vm_hash_func, vm_less_func, NULL);
}

bool insert_vme(struct hash *vm, struct vm_entry *vme)
{
  struct hash_elem *t = hash_insert(vm, &(vme->elem));

  if (t != NULL)
  {
    return true;
  }

  else
  {
    return false;
  }
}

bool delete_vme(struct hash *vm, struct vm_entry *vme)
{
  struct hash_elem *t = hash_delete(vm, &(vme->elem));

  if (t != NULL)
  {
    free_frame(pagedir_get_page(thread_current()->pagedir, vme->addrpn));
    free(t);
    return true;
  }

  else
  {
    return false;
  }
}

struct vm_entry *find_vme(void *vaddr)
{
  struct vm_entry temp;
  temp.addrpn = pg_round_down(vaddr);

  struct thread *cur = thread_current();
  struct hash_elem *e = hash_find(&(cur->vm), &(temp.elem));

  if (e == NULL)
  {
    return NULL;
  }

  else
  {
    return hash_entry(e, struct vm_entry, elem);
  }
}

void vm_destroy(struct hash *vm)
{
  hash_destroy(vm, vm_destructor_func);
}

void vm_load_segment(struct vm_entry *vme, void *uaddr, bool writable, struct file *file, size_t ofs, size_t read_bytes, size_t zero_bytes)
{
  vme->type = 0;
  vme->addrpn = uaddr;
  vme->writable = writable;
  vme->loaded = false;
  vme->file = file;
  vme->offset = ofs;
  vme->read_bytes = read_bytes;
  vme->zero_bytes = zero_bytes;
}

static unsigned vm_hash_func(const struct hash_elem *e, void *aux)
{
  return hash_int(hash_entry(e, struct vm_entry, elem)->addrpn);
}

static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
  return hash_entry(a, struct vm_entry, elem)->addrpn < hash_entry(b, struct vm_entry, elem)->addrpn;
}

static void vm_destructor_func(const struct hash_elem *e, void *aux)
{
  struct vm_entry *temp = hash_entry(e, struct vm_entry, elem);
  struct thread *cur = thread_current();

  free_frame(pagedir_get_page(thread_current()->pagedir, temp->addrpn));
  free(temp);
}

bool load_file(void *kaddr, struct vm_entry *vme)
{
  if (vme->file == NULL)
  {
    return false;
  }
  file_seek(vme->file, vme->offset);
  off_t t = file_read(vme->file, kaddr, vme->read_bytes);
  if (t != vme->read_bytes)
  {
    return false;
  }

  else
  {
    memset(kaddr + t, 0, vme->zero_bytes);
    return true;
  }
}