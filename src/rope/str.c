#define VEC_IMPLEMENTATION
#include <str.h>

#include <string.h>

bool str_eq (struct str * a, struct str * b)
{
    return str_cmp(a, b) == 0;
}

int str_cmp (struct str * a, struct str * b)
{

    return (a == b) ?
        0 :
        (a == NULL) ?
        -1 :
        (b == NULL) ?
        1 :
        (a->ptr == b->ptr) ?
        0 :
        (a->length == b->length) ?
        strncmp(a->ptr, b->ptr, a->length) :
        (a->length < b->length) ?
        -1 :
        1 ;
}
