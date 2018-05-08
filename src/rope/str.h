#ifndef _STR_H
#define _STR_H

#include <tralloc.h>

#define VEC_DATA_TYPE char
#define VEC_PREFIX    str_
#define VEC_VEC       str
#include <utils/vec.h>

bool str_eq  (struct str * a, struct str * b);
int  str_cmp (struct str * a, struct str * b);

#endif /* _STR_H */
