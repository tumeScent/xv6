// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define NSUPERPG 10

static char *super_pages[NSUPERPG];
static int super_used[NSUPERPG];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  
  // reserve a few 2MB for superpages
  for(int i = 0;i < NSUPERPG; i++){
      char *p = (char*)SUPERPGROUNDUP((uint64)end + i *SUPERPGSIZE);
      super_pages[i] = p;
      super_used[i] = 0;
  }

  freerange((void*)SUPERPGROUNDUP((uint64)end + NSUPERPG * SUPERPGSIZE), (void*)PHYSTOP);

  // freerange(end, (void*)PHYSTOP);
}

void *
superalloc(void)
{
  for (int i = 0;i < NSUPERPG;i ++){
    if ( super_used[i] ) continue;
    memset( super_pages[i], 5, SUPERPGSIZE);
    super_used[i] = 1;
    return (void*) super_pages[i];
  }
  return 0;
}

void
superfree(void* pa)
{
  if( (char *)pa < super_pages[0] || (char *)pa > super_pages[NSUPERPG-1]+SUPERPGSIZE || ((uint64)pa % SUPERPGSIZE != 0) )
    panic("superfree: invalid physicall address");
  for( int i = 0;i < NSUPERPG;i ++){
    if( super_pages[i] == pa){
      if( super_used[i] == 0) panic("superfree: double free");
      memset( pa, 1, SUPERPGSIZE);
      super_used[i] = 0;
      return ;
    }
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
