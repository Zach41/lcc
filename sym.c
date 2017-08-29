#include "c.h"

#define equalp(x) v.x == p->sym.u.c.v.x

struct table {
  int level;
  Table previous;
  struct entry {
    struct symbol sym;
    struct entry *link;
  } * buckets[256];
  Symbol all;
};

#define HASHSIZE NELEMS(((Table)0)->buckets)

static struct table cnt = {CONSTANTS}, ext = {GLOBAL}, ids = {GLOBAL},
                    tys = {GLOBAL};

Table constants   = &cnt;
Table externals   = &ext;
Table identifiers = &ids;
Table globals     = &ids;
Table types       = &tys;
Table labels;
int level = GLOBAL;
List loci, symbols;

// create a new table with a specific level
Table table(Table tp, int level) {
  Table new;

  NEW0(new, FUNC);
  new -> previous = tp;
  new -> level = level;
  if (tp) {
    new -> all = tp -> all;
  }
  return new;
}

/*Scanning a table, and apply a function to all the symbol with a specific 
scope in the table. The function is specified by `apply', and `cl' is a
pointer to the user defined data, passing as an argument to `apply' */
void foreach(Table tp,
	     int level,
	     void (*apply)(Symbol, void *),
	     void *cl) {	/* cl is a pointer to the user defined data */
  assert(tp);
  while (tp && tp -> level > level)
    tp = tp -> previous;
  if (tp && tp -> level == level) {
    Symbol p;
    Coordinate sav;
    sav = src;
    for (p = tp -> all; p && p -> scope == level; p = p -> up) {
      src = p -> src;
      apply(p, cl);
    }
    src = sav;
  }
}

void enterscope() {
  ++level;
}

void exitscope() {
  rmtypes(level);
  if (types -> level == level)
    types = types -> previous;
  if (identifiers -> level == level) {
    // TODO: warn if more than 127 identifiers
    identifiers = identifiers -> previous;
  }
  assert(level >= GLOBAL);
  --level;
}

// install a symbol into table, create a table if needed
Symbol install(char *name, Table* tpp, int level, unsigned arena) {
  Table tp = *tpp;
  struct entry *p;

  unsigned h = (unsigned)name & (HASHSIZE - 1);
  if (level > 0 && tp -> level < level)
    tp = *tpp = table(tp, level);
  NEW0(p, arena);
  p -> sym.name = name;
  p -> sym.scope = level;
  p -> sym.up = tp -> all;
  tp -> all = &p -> sym;
  p -> link = tp -> buckets[h];
  tp -> buckets[h] = p;
  return &p -> sym;
}

// lookup a symbol, from bottom to top
Symbol lookup(char *name, Table tp) {
  struct entry *p;
  unsigned h = (unsigned)name & (HASHSIZE - 1);
  do {
    for (p = tp -> buckets[h]; p; p = p -> link)
      if (name == p -> sym.name)
	return &p -> sym;
  } while ((tp = tp -> previous));
}

// generated a label
int genlabel(int n) {
  static int label = 1;
  label += n;
  return label - n;
}

// find a label
Symbol findlabel(int lab) {
  struct entry *p;
  unsigned h = (unsigned)lab & (HASHSIZE - 1);

  for (p = labels -> buckets[h]; p; p = p -> link) {
    if (lab == p -> sym.u.l.label)
      return &p -> sym;
  }
  // not found, build a symbol
  NEW0(p, FUNC);
  p -> sym.name = stringd(lab);
  p -> sym.scope = LABELS;
  p -> sym.up = labels -> all;
  labels -> all = &p -> sym;
  p -> link = labels -> buckets[h];
  labels -> buckets[h] = p;
  p -> sym.generated = 1;
  p -> sym.u.l.label = lab;
  // FIXME: (*IR -> defsymbol)(&p -> sym)
  return &p -> sym;  
}

// find a constant from contant table, install one if not found
Symbol constant(Type ty, Value v) {
  struct entry *p;
  unsigned h = v.u&(HASHSIZE - 1);

  ty = unqual(ty);
  for (p = constants -> buckets[h]; p; p = p -> link) {
    if (eqtype(ty, p -> sym.type, 1))
      // FIXME: return the symbol if p's value == v
      return NULL;
  }
  // install a new constant
  NEW0(p, PERM);
  p -> sym.name = vtoa(ty, v);
  p -> sym.scope = CONSTANTS;
  p -> sym.type = ty;
  // FIXME: uncommet this
  /* p -> sym.sclass = STATIC; */
  p -> sym.u.c.v = v;
  p -> link = constants -> buckets[h];
  constants -> buckets[h] = p;
  p -> sym.up = constants -> all;
  constants -> all = &p -> sym;
  // TODO: announce the constant, if necessary
  p -> sym.defined = 1;
  return &p -> sym;
}

// function to generated local variable used in switch stmt, args passing in function, etc.
Symbol genident(int scls, Type ty, int level) {
  Symbol p;
  NEW0(p, level >= LOCAL ? FUNC : PERM);
  p -> name = stringd(genlabel(1));
  p -> scope = level;
  p -> sclass = scls;
  p -> type = ty;
  p -> generated = 1;
  if (level == GLOBAL)
    // FIXME: (*IR -> defsymbol)(p)
    assert(1);
  return p;  
}

// generated a temporary symbol
Symbol temporary(int scls, Type ty, int level) {
  Symbol p = genident(scls, ty, level);
  p -> temporary = 1;
  return p;
}

// generated a temporary symbol, used by the backend of compiler
/* Symbol newtemp(int sclass, int tc) {} */


