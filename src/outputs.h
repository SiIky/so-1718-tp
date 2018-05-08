#ifndef _OUTPUTS_H
#define _OUTPUTS_H

#include <tralloc.h>
#include <stddef.h>

struct outputs {
    size_t i;
    size_t f;
};

#define VEC_DATA_TYPE_EQ(L, R) (((L).i == (R).i) && ((L).f == (R).f))
#define VEC_DATA_TYPE struct outputs
#define VEC_VEC ovec
#define VEC_PREFIX ovec_
#include <utils/vec.h>

#endif /* _OUTPUTS_H */
