#ifndef LCC_HEADER_H
#define LCC_HEADER_H

#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define NELEMS(array) ((int)(sizeof(array) / sizeof((array)[0])))

#define roundup(x, n) (((x) + (n) - 1) & (~((n) - 1)))

enum { PERM=0, FUNC, STMT };

#define NEW(p, a) ((p) = allocate(sizeof(*(p)), (a)))
#define NEW0(p, a) memset(NEW((p), (a)), 0, sizeof(*(p)))

#endif /* LCC_HEADER_H */
