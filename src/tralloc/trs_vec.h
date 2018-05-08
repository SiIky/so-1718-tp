/*
 * "trs.h"
 *  _trs_free()
 *  struct trs
 *  trs_eq()
 */
#include "trs.h"

#undef _VEC_CALLOC
#undef _VEC_FREE
#undef _VEC_MALLOC
#undef _VEC_REALLOC

#define VEC_DATA_TYPE    struct trs
#define VEC_DATA_TYPE_EQ trs_eq
#define VEC_DTOR         _trs_free
#define VEC_PREFIX       trs_
#include <utils/vec.h>
