#include <rope.h>

#define VEC_IMPLEMENTATION
#define VEC_DATA_TYPE          struct str *
#define VEC_DATA_TYPE_EQ(A, B) (str_eq((A), (B)))
#define VEC_DTOR               str_free
#define VEC_PREFIX             rope_
#define VEC_VEC                rope
#include <utils/vec.h>
