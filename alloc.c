#include "c.h"

struct block {
  struct block *next;
  char *limit;
  char *avail;
};

union align {
  long l;
  char *p;
  double d;
  int (*f)(void);
};

union header {
  struct block b;
  union align a;
};

#ifdef PURIFY
static union header *arena[3];

void* allocate(unsigned long n, unsigned a) {
  assert(a < NELEMS(arena));
  
  union header *new = malloc(sizeof(*new) + n);
  if (new == NULL) {
    fprintf(stderr, "insufficient memory\n");
    exit(1);
  }
  new -> b.next = (void *)arena[a];
  arena[a] = new;
  return new + 1;
}

void deallocate(unsigned a) {
  assert(a < NELEMS(arena));

  union header *p, *q;

  for (p = arena[a]; p; p = q) {
    q = (void *)p -> next;
    free(p);
  }
  arena[a] = NULL;
}

void *newarray(unsigned long m, unsigned long n, unsigned a) {
  return allocate(m*n, a);
}
#else
static struct block
  first[] = { {NULL}, {NULL}, {NULL}},
  *arena[] = {&first[0], &first[1], &first[2]};

static struct block *freeblocks;

void *allocate(unsigned long n, unsigned a) {
  assert(a < NELEMS(arena));
  assert(n > 0);

  struct block *ap = arena[a];
  n = roundup(n, sizeof(union align));
  while (n > ap -> limit - ap -> avail) {
    if ((ap -> next = freeblocks) != NULL) {
      freeblocks = freeblocks -> next;
      ap = ap -> next;
    } else {
      unsigned m = sizeof(union header) + n
	+ roundup(10*1024, sizeof(union align));
      ap -> next = malloc(m);
      ap = ap -> next;
      if (ap == NULL) {
	fprintf(stderr, "insufficient memory\n");
	exit(1);
      }
      ap -> limit = (char*)ap + m;

      ap -> avail = (char*)((union header*)ap + 1);
      ap -> next = NULL;
      arena[a] = ap;
    }
  }
  ap -> avail += n;
  return ap -> avail - n;
}

void deallocate(unsigned a) {
  assert(a < NELEMS(arena));

  arena[a] -> next = freeblocks;
  freeblocks = first[a].next;
  first[a].next = NULL;
  arena[a] = &first[a];
}

void *newarray(unsigned long m, unsigned n, unsigned a) {
  return allocate(m*n, a);
}
#endif
