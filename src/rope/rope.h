#ifndef _ROPE_H
#define _ROPE_H

#include <tralloc.h>

#define VEC_DATA_TYPE          struct str *
#define VEC_DATA_TYPE_EQ(A, B) (str_eq((A), (B)))
#define VEC_DTOR               str_free
#define VEC_PREFIX             rope_
#define VEC_VEC                rope
#include <utils/vec.h>

#endif /* _ROPE_H */
