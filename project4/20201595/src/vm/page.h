#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <list.h>
#include "userprog/pagedir.h"
#include <stdbool.h>

struct vm_entry
{
  uint8_t type; // 0: bin, 1: file , 2: swap
  void *addrpn; // vaddres page number
  bool writable;
  bool loaded;
  struct file *file;

  size_t offset;
  size_t read_bytes;
  size_t zero_bytes;
  size_t swap_slot;

  struct hash_elem elem;
  struct list_elem mmap_elem;
};

struct frame
{
  void *kaddr;
  struct vm_entry *vme;
  struct thread *thread;
  struct list_elem lru;
};

void vm_init(struct hash *vm);
bool insert_vme(struct hash *vm, struct vm_entry *vme);
bool delete_vme(struct hash *vm, struct vm_entry *vme);
struct vm_entry *find_vme(void *vaddr);
void vm_destroy(struct hash *vm);
void vm_load_segment(struct vm_entry *vme, void *uaddr, bool writable,
                     struct file *file, size_t ofs, size_t read_bytes, size_t zero_bytes);

bool load_file(void *kaddr, struct vm_entry *vme);

static unsigned vm_hash_func(const struct hash_elem *e, void *aux);
static bool vm_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux);
static void vm_destructor_func(const struct hash_elem *e, void *aux);
#endif