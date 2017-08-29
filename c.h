#ifndef LCC_HEADER_H
#define LCC_HEADER_H

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NELEMS(array) ((int)(sizeof(array) / sizeof((array)[0])))

#define roundup(x, n) (((x) + (n) - 1) & (~((n) - 1)))
// FIXME: isqual(t) ((t) -> op >= CONST)
#define isqual(t) false
// FIXME: unqual(t) (isqual(t) ? (t) -> type : (t))
#define unqual(t) (t)

enum { PERM=0, FUNC, STMT };

#define NEW(p, a) ((p) = allocate(sizeof(*(p)), (a)))
#define NEW0(p, a) memset(NEW((p), (a)), 0, sizeof(*(p)))

typedef struct coord {
  char *file;			/* filename */
  unsigned x, y;		/* line number and char number */
} Coordinate;

typedef union value {
  char sc;			/* signed char */
  short ss;
  int i;
  unsigned char uc;
  unsigned short us;
  unsigned int u;
  float f;
  double d;
  void *p;
} Value;

typedef struct table *Table;

typedef struct symbol *Symbol;

typedef struct type *Type;

typedef struct list *List;

typedef struct field *Field;

typedef struct node *Node;

enum { CONSTANTS=1, LABELS, GLOBAL, PARAM, LOCAL };

struct symbol {
  char *name;
  int scope;
  Coordinate src;
  Symbol up;
  List uses;
  int sclass;
  
  unsigned structarg:1;
  unsigned addressed:1;
  unsigned temporary:1;
  unsigned generated:1;
  unsigned defined:1;

  Type type;
  float ref;

  union {
    struct {
      int label;
      Symbol equatedto;
    } l;
    struct {
      unsigned cfield:1;
      unsigned vfiled:1;
      Table ftab;
      Field flist;
    } s;
    int value;
    Symbol *idlist;
    struct {
      Value v;
      Symbol loc;
    } c;
    struct {
      Coordinate pt;
      int label;
      int ncalls;
      Symbol *callee;
    } f;
    int seg;
    struct {
      Node cse;
    } t;
  } u;
};

extern Coordinate src;		/* current src position */

extern void rmtypes(int);
extern int eqtype(Type, Type, int);
extern char* vtoa(Type, Value);

#endif /* LCC_HEADER_H */
