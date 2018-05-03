/* vec - v2018.05.03-1
 *
 * A vector type inspired by
 *  * Rust's `Vec` type
 *  * [stb](https://github.com/nothings/stb)
 *  * [sort](https://github.com/swenson/sort)
 *
 * The most up to date version of this file can be found at
 * `include/vec.h` on [siiky/c-utils](https://github.com/siiky/c-utils)
 * More usage examples can be found at `src/vec` on the link above
 *
 * # Usage
 *
 * ```c
 * #include <assert.h>
 *
 * #define VEC_DATA_TYPE size_t
 *
 * // Optionally, define an equality function
 * // (Not optional for compound types, like structs)
 * #define VEC_DATA_TYPE_EQ(L, R) ((L) == (R))
 *
 * // Optionally, define a destructor function of type
 * //     VEC_DATA_TYPE dtor (VEC_DATA_TYPE);
 * // If defined, it will be used by VEC_FILTER() and VEC_FREE()
 * #define VEC_DTOR my_dtor_function
 *
 * // Optionally, define a prefix (defaults to `vec_`)
 * #define VEC_PREFIX my_
 *
 * // Optionally, define the struct identifier
 * #define VEC_VEC my_uber_vec
 *
 * // Optionally, define NDEBUG to disable asserts inside vec.h
 * #define NDEBUG
 *
 * // Optionally, define VEC_STATIC to mark definitions as static
 * // Probably will raise `unused-function` warnings
 * #define VEC_STATIC
 *
 * #include <vec.h>
 *
 * int main (void)
 * {
 *     struct my_uber_vec * vec = my_new();
 *
 *     assert(vec != NULL);
 *
 *     size_t used = 0;
 *     for (size_t i = 0; i < 100; i++)
 *         if (my_push(vec, i))
 *             used++;
 *
 *     {
 *         size_t len = my_len(vec);
 *         size_t cap = my_capacity(vec);
 *
 *         assert(used == len);
 *         assert(cap >= len);
 *     }
 *
 *     for (my_iter(vec); my_itering(vec); my_iter_next(vec)) {
 *         size_t r = my_get_nth(vec, my_iter_idx(vec));
 *         assert(r == i);
 *     }
 *
 *     // If VEC_DTOR is defined, VEC_FREE() will automatically
 *     // call it on every element
 *     vec = my_free(vec);
 *
 *     {
 *         // For VEC_LEN(), VEC_CAPACITY(), VEC_AS_MUT_SLICE(),
 *         // VEC_AS_SLICE() and VEC_IS_EMPTY() NULL is an empty vector
 *         bool empty = my_is_empty(vec);
 *         size_t * ptr = my_as_slice(vec);
 *         size_t cap = my_capacity(vec);
 *         size_t len = my_len(vec);
 *
 *         assert(cap == 0);
 *         assert(empty);
 *         assert(len == 0);
 *         assert(ptr == NULL);
 *         assert(vec == NULL);
 *     }
 *
 *     return 0;
 * }
 * ```
 *
 * # TODO
 *
 * - [X] Allow more than one implementation for different types
 * - [ ] More options
 */

/*
 * <assert.h>
 *  assert()
 *
 * <stdbool.h>
 *  bool
 *  false
 *  true
 *
 * <stddef.h>
 *  size_t
 *
 * <stdlib.h>
 *  bsearch()
 *  calloc()
 *  free()
 *  malloc()
 *  qsort()
 *  realloc()
 *
 * <string.h>
 *  memcpy()
 */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*
 * Type of data for the vector to hold
 */
# ifndef VEC_DATA_TYPE
#  error "Must declare VEC_DATA_TYPE"
# endif /* VEC_DATA_TYPE */

/*
 * If no prefix was defined, default to `vec_`
 */
# ifndef VEC_PREFIX
#  define VEC_PREFIX vec_
#  define VEC_VEC    vec
# endif /* VEC_PREFIX */

/*
 * Magic from `sort.h`
 */
# define VEC_CONCAT(A, B)    A ## B
# define VEC_MAKE_STR1(A, B) VEC_CONCAT(A, B)
# define VEC_MAKE_STR(A)     VEC_MAKE_STR1(VEC_PREFIX, A)

/*
 * Allow overwriting the vector name
 */
# ifndef VEC_VEC
#  define VEC_VEC VEC_MAKE_STR(vec)
# endif

# ifdef VEC_IMPLEMENTATION

/**=========================================================
 * @brief The vector type
 */
struct VEC_VEC {
    /** Pointer to actual data */
    VEC_DATA_TYPE * ptr;

    /** Number of elements */
    size_t length;

    /** Number of elements the vector can hold currently */
    size_t capacity;

    /** Iterator index */
    size_t idx;

    /** Should iterate from begginning or end */
    unsigned char reverse : 1;

    /** Is currently iterating */
    unsigned char iterating : 1;
};

# else

struct VEC_VEC;

# endif /* VEC_IMPLEMENTATION */

# ifndef VEC_DATA_TYPE_EQ
#  define VEC_DATA_TYPE_EQ(L, R) ((L) == (R))
# endif /* VEC_DATA_TYPE_EQ */

# ifdef VEC_STATIC
#  undef VEC_STATIC
#  define VEC_STATIC static
# else /* VEC_STATIC */
#  undef VEC_STATIC
#  define VEC_STATIC
# endif /* VEC_STATIC */

/*==========================================================
 * Function names
 *=========================================================*/
#define VEC_APPEND             VEC_MAKE_STR(append)
#define VEC_AS_MUT_SLICE       VEC_MAKE_STR(as_mut_slice)
#define VEC_AS_SLICE           VEC_MAKE_STR(as_slice)
#define VEC_BSEARCH            VEC_MAKE_STR(bsearch)
#define VEC_CAPACITY           VEC_MAKE_STR(capacity)
#define VEC_ELEM               VEC_MAKE_STR(elem)
#define VEC_FILTER             VEC_MAKE_STR(filter)
#define VEC_FIND               VEC_MAKE_STR(find)
#define VEC_FREE               VEC_MAKE_STR(free)
#define VEC_FREE_RANGE         VEC_MAKE_STR(free_range)
#define VEC_FROM_RAW_PARTS     VEC_MAKE_STR(from_raw_parts)
#define VEC_GET_NTH            VEC_MAKE_STR(get_nth)
#define VEC_INSERT             VEC_MAKE_STR(insert)
#define VEC_IS_EMPTY           VEC_MAKE_STR(is_empty)
#define VEC_ITER               VEC_MAKE_STR(iter)
#define VEC_ITERING            VEC_MAKE_STR(itering)
#define VEC_ITER_END           VEC_MAKE_STR(iter_end)
#define VEC_ITER_IDX           VEC_MAKE_STR(iter_idx)
#define VEC_ITER_NEXT          VEC_MAKE_STR(iter_next)
#define VEC_ITER_REV           VEC_MAKE_STR(iter_rev)
#define VEC_LEN                VEC_MAKE_STR(len)
#define VEC_MAP                VEC_MAKE_STR(map)
#define VEC_MAP_RANGE          VEC_MAKE_STR(map_range)
#define VEC_NEW                VEC_MAKE_STR(new)
#define VEC_POP                VEC_MAKE_STR(pop)
#define VEC_PUSH               VEC_MAKE_STR(push)
#define VEC_QSORT              VEC_MAKE_STR(qsort)
#define VEC_REMOVE             VEC_MAKE_STR(remove)
#define VEC_RESERVE            VEC_MAKE_STR(reserve)
#define VEC_SET_LEN            VEC_MAKE_STR(set_len)
#define VEC_SET_NTH            VEC_MAKE_STR(set_nth)
#define VEC_SHRINK_TO_FIT      VEC_MAKE_STR(shrink_to_fit)
#define VEC_SPLIT_OFF          VEC_MAKE_STR(split_off)
#define VEC_SWAP_REMOVE        VEC_MAKE_STR(swap_remove)
#define VEC_TRUNCATE           VEC_MAKE_STR(truncate)
#define VEC_WITH_CAPACITY      VEC_MAKE_STR(with_capacity)
#define _VEC_CHANGE_CAPACITY   VEC_MAKE_STR(_change_capacity)
#define _VEC_DECREASE_CAPACITY VEC_MAKE_STR(_decrease_capacity)
#define _VEC_INCREASE_CAPACITY VEC_MAKE_STR(_increase_capacity)

/*==========================================================
 * Function prototypes
 *
 * RETURN TYPE        FUNCTION NAME      PARAMETER LIST
 *==========================================================*/
VEC_DATA_TYPE         VEC_GET_NTH        (const struct VEC_VEC * self, size_t nth);
VEC_DATA_TYPE         VEC_POP            (struct VEC_VEC * self);
VEC_DATA_TYPE         VEC_REMOVE         (struct VEC_VEC * self, size_t index);
VEC_DATA_TYPE         VEC_SWAP_REMOVE    (struct VEC_VEC * self, size_t index);
VEC_DATA_TYPE *       VEC_AS_MUT_SLICE   (struct VEC_VEC * self);
bool                  VEC_APPEND         (struct VEC_VEC * restrict self, struct VEC_VEC * restrict other);
bool                  VEC_ELEM           (const struct VEC_VEC * self, VEC_DATA_TYPE element);
bool                  VEC_FILTER         (struct VEC_VEC * self, bool pred (VEC_DATA_TYPE *));
bool                  VEC_FREE_RANGE     (struct VEC_VEC * self, size_t from, size_t to);
bool                  VEC_INSERT         (struct VEC_VEC * self, size_t index, VEC_DATA_TYPE element);
bool                  VEC_IS_EMPTY       (const struct VEC_VEC * self);
bool                  VEC_ITER           (struct VEC_VEC * self);
bool                  VEC_ITERING        (const struct VEC_VEC * self);
bool                  VEC_ITER_END       (struct VEC_VEC * self);
bool                  VEC_ITER_NEXT      (struct VEC_VEC * self);
bool                  VEC_ITER_REV       (struct VEC_VEC * self, bool rev);
bool                  VEC_MAP            (struct VEC_VEC * self, VEC_DATA_TYPE f (VEC_DATA_TYPE));
bool                  VEC_MAP_RANGE      (struct VEC_VEC * self, VEC_DATA_TYPE f (VEC_DATA_TYPE), size_t from, size_t to);
bool                  VEC_PUSH           (struct VEC_VEC * self, VEC_DATA_TYPE element);
bool                  VEC_QSORT          (struct VEC_VEC * self, int compar (const void *, const void *));
bool                  VEC_RESERVE        (struct VEC_VEC * self, size_t total);
bool                  VEC_SET_LEN        (struct VEC_VEC * self, size_t len);
bool                  VEC_SET_NTH        (struct VEC_VEC * self, size_t nth, VEC_DATA_TYPE element);
bool                  VEC_SHRINK_TO_FIT  (struct VEC_VEC * self);
bool                  VEC_TRUNCATE       (struct VEC_VEC * self, size_t len);
const VEC_DATA_TYPE * VEC_AS_SLICE       (const struct VEC_VEC * self);
size_t                VEC_BSEARCH        (const struct VEC_VEC * self, VEC_DATA_TYPE key, int compar (const void *, const void *));
size_t                VEC_CAPACITY       (const struct VEC_VEC * self);
size_t                VEC_FIND           (const struct VEC_VEC * self, VEC_DATA_TYPE element);
size_t                VEC_ITER_IDX       (struct VEC_VEC * self);
size_t                VEC_LEN            (const struct VEC_VEC * self);
struct VEC_VEC *      VEC_FREE           (struct VEC_VEC * self);
struct VEC_VEC *      VEC_FROM_RAW_PARTS (VEC_DATA_TYPE * ptr, size_t length, size_t capacity);
struct VEC_VEC *      VEC_NEW            (void);
struct VEC_VEC *      VEC_SPLIT_OFF      (struct VEC_VEC * self, size_t at);
struct VEC_VEC *      VEC_WITH_CAPACITY  (size_t capacity);

#ifdef VEC_IMPLEMENTATION

# ifndef _VEC_MALLOC
#  define _VEC_MALLOC malloc
# endif

# ifndef _VEC_CALLOC
#  define _VEC_CALLOC calloc
# endif

# ifndef _VEC_REALLOC
#  define _VEC_REALLOC realloc
# endif

# ifndef _VEC_FREE
#  define _VEC_FREE free
# endif

/*==========================================================
 * Function definitions
 *========================================================*/

/**=========================================================
 * @brief Try to change the capacity of @a self to @a cap
 * @param self The vector
 * @param cap The new capacity
 * @returns `true` if the operation was successful, `false` otherwise
 */
static inline bool _VEC_CHANGE_CAPACITY (struct VEC_VEC * self, size_t cap)
{
    VEC_DATA_TYPE * ptr = _VEC_REALLOC(self->ptr, cap * sizeof(VEC_DATA_TYPE));

    if (ptr != NULL) {
        self->ptr = ptr;
        self->capacity = cap;
    }

    return ptr != NULL;
}

/**=========================================================
 * @brief Check if @a self has capacity for another element, and try
 *        to increase it
 * @param self The vector
 * @returns `true` if @a self has enough space for at least one more
 *          element, `false` otherwise
 */
static inline bool _VEC_INCREASE_CAPACITY (struct VEC_VEC * self)
{
    if (self == NULL)
        return false;

    if (self->length < self->capacity)
        return true;

    /* new_cap = (cap * 1.5) + 1 */
    size_t new_cap = self->capacity + (self->capacity >> 1) + 1;

    return VEC_RESERVE(self, new_cap);
}

/**=========================================================
 * @brief Check if @a self has too much unused memory and decrease it
 * @param self The vector
 * @returns `false` if @a self has too much unused memory but it
 *          couldn't decrease it, `true` otherwise
 */
static inline bool _VEC_DECREASE_CAPACITY (struct VEC_VEC * self)
{
    /* more than half of the capacity in use? */
    if (self->length > (self->capacity >> 1))
        return true;

    /* new_cap = len * 1.5 */
    /* len * 1.5 ~ cap * 0.75 */
    size_t new_cap = self->length + (self->length >> 1);
    return _VEC_CHANGE_CAPACITY(self, new_cap);
}

/**=========================================================
 * @brief Free @a self
 * @param self The vector
 * @returns `NULL`
 */
VEC_STATIC struct VEC_VEC * VEC_FREE (struct VEC_VEC * self)
{
    if (self != NULL) {
# ifdef VEC_DTOR
        VEC_FREE_RANGE(self, 0, self->length);
# endif /* VEC_DTOR */

        if (self->ptr != NULL)
            _VEC_FREE(self->ptr);

        _VEC_FREE(self);
    }

    return NULL;
}

/**=========================================================
 * @brief Create a new empty vector
 * @returns The new vector
 */
VEC_STATIC inline struct VEC_VEC * VEC_NEW (void)
{
    return VEC_FROM_RAW_PARTS(NULL, 0, 0);
}

/**=========================================================
 * @brief Create a new vector with @a capacity free slots
 * @param capacity Number of elements to allocate
 * @returns The new vector
 */
VEC_STATIC inline struct VEC_VEC * VEC_WITH_CAPACITY (size_t capacity)
{
    if (capacity == 0)
        return VEC_NEW();

    VEC_DATA_TYPE * ptr = _VEC_CALLOC(capacity, sizeof(VEC_DATA_TYPE));
    if (ptr == NULL)
        return NULL;

    struct VEC_VEC * ret = VEC_FROM_RAW_PARTS(ptr, 0, capacity);
    if (ret == NULL)
        _VEC_FREE(ptr);

    return ret;
}

/**=========================================================
 * @brief Create a vector from separate components. In case it is not
 *        possible to allocate memory for the vector, nothing is done
 *        to @a ptr
 * @param ptr A pointer to allocated memory
 * @param length Number of elements in @a ptr
 * @param capacity Total number of elements @a ptr can hold
 * @returns A new vector pointing to @a ptr, with @a length and @a capacity
 */
VEC_STATIC inline struct VEC_VEC * VEC_FROM_RAW_PARTS (VEC_DATA_TYPE * ptr, size_t length, size_t capacity)
{
    struct VEC_VEC * ret = _VEC_MALLOC(sizeof(struct VEC_VEC));

    if (ret != NULL) {
        ret->ptr = ptr;
        ret->length = length;
        ret->capacity = capacity;

        ret->idx = 0;
        ret->reverse = 0;
        ret->iterating = 0;
    }

    return ret;
}

/**=========================================================
 * @brief Calculate the capacity of @a self
 * @param self The vector
 * @returns The capacity of @a self
 */
VEC_STATIC inline size_t VEC_CAPACITY (const struct VEC_VEC * self)
{
    return (self != NULL) ?
        self->capacity :
        0;
}

/**=========================================================
 * @brief Reserve memory for @a total elements
 * @param self The vector
 * @param total Number of total elements
 * @returns `true` if @a self already had enough capacity or it was
 *          able to reserve the extra capacity needed, `false` otherwise
 */
VEC_STATIC bool VEC_RESERVE (struct VEC_VEC * self, size_t total)
{
    return (self == NULL) ?
        false :
        (self->capacity >= total) ?
        true :
        _VEC_CHANGE_CAPACITY(self, total);
}

/**=========================================================
 * @brief Shrink the capacity of @a self to its length
 * @param self The vector
 * @returns `false` if it wasn't possible to shrink @a self,
 *          `true` otherwise
 */
VEC_STATIC bool VEC_SHRINK_TO_FIT (struct VEC_VEC * self)
{
    return self != NULL && _VEC_CHANGE_CAPACITY(self, self->length);
}

/**=========================================================
 * @brief Shorten @a self, keeping the first @a len elements
 * @param self The vector
 * @param len New length
 * @returns `false` if @a self is not a valid vector, `true` otherwise
 */
VEC_STATIC bool VEC_TRUNCATE (struct VEC_VEC * self, size_t len)
{
    if (self == NULL)
        return false;

    if (self->length > len) {
        VEC_FREE_RANGE(self, len, self->length);
        self->length = len;
    }

    return true;
}

/**=========================================================
 * @brief Return a `const` qualified pointer to allocated memory
 * @param self The vector
 * @returns `const` qualified pointer to allocated memory
 */
VEC_STATIC inline const VEC_DATA_TYPE * VEC_AS_SLICE (const struct VEC_VEC * self)
{
    return (self != NULL) ?
        (const VEC_DATA_TYPE *) self->ptr :
        NULL;
}

/**=========================================================
 * @brief Like vec_as_slice, but not `const` qualified
 * @param self The vector
 * @returns Pointer to allocated memory
 */
VEC_STATIC inline VEC_DATA_TYPE * VEC_AS_MUT_SLICE (struct VEC_VEC * self)
{
    return (self != NULL) ?
        self->ptr :
        NULL;
}

/**=========================================================
 * @brief Set the length of @a self to @a len, without checks
 * @param self The vector
 * @param len The new length
 * @returns `false` if @a self not a valid vector, `true` otherwise
 */
VEC_STATIC inline bool VEC_SET_LEN (struct VEC_VEC * self, size_t len)
{
    if (self == NULL)
        return false;
    self->length = len;
    return true;
}

/**=========================================================
 * @brief Remove the element at @a index, replacing it with the last
 *        element
 * @param self The vector
 * @param index Index of the element to be removed
 * @returns The removed element
 */
VEC_STATIC VEC_DATA_TYPE VEC_SWAP_REMOVE (struct VEC_VEC * self, size_t index)
{
    assert(self != NULL);
    assert(self->ptr != NULL);
    assert(self->length > 0);
    assert(index < self->length);

    VEC_DATA_TYPE ret = self->ptr[index];
    self->ptr[index] = VEC_POP(self);

    return ret;
}

/**=========================================================
 * @brief Insert an @a element at @a index, shifting every element
 *        before it to the right
 * @param self The vector
 * @param index Where the element will be inserted
 * @param element Element to be inserted
 * @returns `false` if @a index is out of bounds or @a self didn't have
 *          enough capacity and it wasn't possible to increase it,
 *          `true` otherwise
 */
VEC_STATIC bool VEC_INSERT (struct VEC_VEC * self, size_t index, VEC_DATA_TYPE element)
{
    if (self == NULL
    /* allow inserting at the head */
    || (VEC_IS_EMPTY(self) && index > 0)
    || index >= self->length
    || !_VEC_INCREASE_CAPACITY(self))
        return false;

    for (size_t i = self->length; i >= index; i--)
        self->ptr[i] = self->ptr[i - 1];

    self->length++;
    self->ptr[index] = element;

    return true;
}

/**=========================================================
 * @brief Remove an element at @a index, shifting every element after
 *        it to the left
 * @param self The vector
 * @param index Index of the element to remove
 * @returns The removed element
 */
VEC_STATIC VEC_DATA_TYPE VEC_REMOVE (struct VEC_VEC * self, size_t index)
{
    assert(self != NULL);
    assert(self->ptr != NULL);
    assert(index < self->length);

    VEC_DATA_TYPE ret = self->ptr[index];

    for (size_t i = index + 1; i < self->length; i++)
        self->ptr[i - 1] = self->ptr[i];

    self->length--;

    _VEC_DECREASE_CAPACITY(self);

    return ret;
}

/**=========================================================
 * @brief Free the elements of @a self in the range [@a from, @a to[
 * @param self The vector
 * @param from The start index
 * @param to The end index (not including element at this index)
 * @returns Same as VEC_MAP_RANGE()
 */
bool VEC_FREE_RANGE (struct VEC_VEC * self, size_t from, size_t to)
{
# ifdef VEC_DTOR
    return VEC_MAP_RANGE(self, VEC_DTOR, from, to);
# else
    /* Suppress unused warnings */
    (void) self;
    (void) from;
    (void) to;
    return true;
# endif
}

/**=========================================================
 * @brief Keep every element of @a self that satisfies a predicate
 *        @a pred
 * @param self The vector
 * @param pred The predicate
 * @returns `false` if @a self is not a valid vector or @a pred is
 *          NULL, `true` otherwise
 */
VEC_STATIC bool VEC_FILTER (struct VEC_VEC * self, bool pred (VEC_DATA_TYPE *))
{
    if (self == NULL
    || self->ptr == NULL
    || pred == NULL)
        return false;

    size_t len = 0;

    for (size_t r = 0; r < self->length; r++)
        if (pred(self->ptr + r))
            self->ptr[len++] = self->ptr[r];
# ifdef VEC_DTOR
        else
            VEC_DTOR(self->ptr[r]);
# endif /* VEC_DTOR */

    self->length = len;

    _VEC_DECREASE_CAPACITY(self);

    return true;
}

/**=========================================================
 * @brief Insert an @a element at the end of @a self
 * @param self The vector
 * @param element Element to be pushed
 * @returns `false` if @a self is not a valid vector, or it didn't
 *          have enough capacity and it wasn't possible to increase
 *          it, `true` otherwise
 */
VEC_STATIC bool VEC_PUSH (struct VEC_VEC * self, VEC_DATA_TYPE element)
{
    if (self == NULL || !_VEC_INCREASE_CAPACITY(self))
        return false;

    self->ptr[self->length] = element;
    self->length++;

    return true;
}

/**=========================================================
 * @brief Remove the last element of @a self
 * @param self The vector
 * @returns The removed element
 */
VEC_STATIC VEC_DATA_TYPE VEC_POP (struct VEC_VEC * self)
{
    assert(self != NULL);
    assert(self->ptr != NULL);
    assert(self->length > 0);

    self->length--;

    _VEC_DECREASE_CAPACITY(self);

    return self->ptr[self->length];
}

/**=========================================================
 * @brief Append @a other to @a self and set the length of @a other
 *        to 0
 * @param self The vector
 * @param other The other vector
 * @returns `false` if either @a self or @a other aren't valid vectors,
 *          @a self didn't have enough capacity and it wasn't possible
 *          to increase it, `true` otherwise
 */
VEC_STATIC bool VEC_APPEND (struct VEC_VEC * restrict self, struct VEC_VEC * restrict other)
{
    if (self == NULL
    || other == NULL
    || self->ptr == NULL
    || other->ptr == NULL
    || self->ptr == other->ptr
    || !VEC_RESERVE(self, self->length + other->length))
        return false;

    VEC_DATA_TYPE * dest = self->ptr + self->length;
    VEC_DATA_TYPE * src = other->ptr;
    size_t n = other->length * sizeof(VEC_DATA_TYPE);
    memcpy(dest, src, n);

    other->length = 0;

    return true;
}

/**=========================================================
 * @brief Calculate the length of @a self
 * @param self The vector
 * @returns The length of @a self
 */
VEC_STATIC inline size_t VEC_LEN (const struct VEC_VEC * self)
{
    return (self != NULL) ?
        self->length :
        0;
}

/**=========================================================
 * @brief Check if @a self is empty
 * @param self The vector
 * @returns `true` if @a self is empty, `false` otherwise
 */
VEC_STATIC inline bool VEC_IS_EMPTY (const struct VEC_VEC * self)
{
    return self == NULL
        || self->ptr == NULL
        || self->length == 0;
}

/**=========================================================
 * @brief Split @a self in two, keeping the first `at - 1` elements
 *        in @a self, and the rest in the returned vector
 * @param self The vector
 * @param at Where to split
 * @returns The new vector
 */
VEC_STATIC struct VEC_VEC * VEC_SPLIT_OFF (struct VEC_VEC * self, size_t at)
{
    if (self == NULL
    || self->ptr == NULL
    || at >= self->length
    || at <= 0)
        return NULL;

    struct VEC_VEC * ret = VEC_WITH_CAPACITY(self->length - at + 1);

    if (ret == NULL)
        return NULL;

    if (ret->ptr != NULL) {
        void * dest = ret->ptr;
        const void * src = self->ptr + at - 1;
        size_t n = sizeof(VEC_DATA_TYPE) * ret->capacity;
        memcpy(dest, src, n);

        ret->length = ret->capacity;
        self->length -= ret->capacity;
    }

    return ret;
}

/**=========================================================
 * @brief Get the element at the @a nth index
 * @param self The vector
 * @param nth The index
 * @returns The element at index @a nth
 */
VEC_STATIC inline VEC_DATA_TYPE VEC_GET_NTH (const struct VEC_VEC * self, size_t nth)
{
    assert(self != NULL);
    assert(self->ptr != NULL);
    assert(nth < self->length);
    return self->ptr[nth];
}

/**=========================================================
 * @brief Set the element at index @a nth to @a element
 * @param self The vector
 * @param nth The index
 * @param element The new element
 * @returns `false` if @a self is not a valid vector or @a nth is out
 *          of bounds, `true` otherwise
 */
VEC_STATIC inline bool VEC_SET_NTH (struct VEC_VEC * self, size_t nth, VEC_DATA_TYPE element)
{
    if (self == NULL
    || self->ptr == NULL
    || nth >= self->length)
        return false;
    self->ptr[nth] = element;
    return true;
}

/**=========================================================
 * @brief Get the index of the first occurrence (if any) of @a element
 * @param self The vector
 * @param element The element to look for
 * @returns The index of the first occurrence of @a element, or, if
 *          @a element does not exist, the length of @a self
 */
VEC_STATIC size_t VEC_FIND (const struct VEC_VEC * self, VEC_DATA_TYPE element)
{
    assert(self != NULL);
    assert(self->ptr != NULL);

    size_t ret = 0;
    for (ret = 0;
            ret < self->length
            && !(VEC_DATA_TYPE_EQ(self->ptr[ret], element));
            ret++);

    return ret;
}

/**=========================================================
 * @brief Check if @a self contains an @a element
 * @param self The vector
 * @param element The element to look for
 * @returns `true` if @a element exists in @a self, `false` otherwise
 */
VEC_STATIC inline bool VEC_ELEM (const struct VEC_VEC * self, VEC_DATA_TYPE element)
{
    return self != NULL
        && self->ptr != NULL
        && (VEC_FIND(self, element) < self->length);
}

/**=========================================================
 * @brief Wraper for `stdlib.h`'s `bsearch()` function
 * @param self The vector
 * @param key The key to look for
 * @param compar A function suitable to be passed to `bsearch()`
 * @returns The index of an occurrence of @a element, or, if @a element
 *          does not exist, the length of @a self
 */
VEC_STATIC size_t VEC_BSEARCH (const struct VEC_VEC * self, VEC_DATA_TYPE key, int compar (const void *, const void *))
{
    assert(self != NULL);
    assert(self->ptr != NULL);
    assert(compar != NULL);

    const void * base = self->ptr;
    size_t nmemb = self->length;
    size_t size = sizeof(VEC_DATA_TYPE);

    VEC_DATA_TYPE * found = bsearch(&key, base, nmemb, size, compar);
    return (found != NULL) ?
        (size_t) (found - self->ptr) :
        self->length;
}

/**=========================================================
 * @brief Wraper for `stdlib.h`'s `qsort()` function
 * @param self The vector
 * @param compar A function suitable to be passed to `qsort()`
 * @returns `false` if @a self is not a valid vector, `true` otherwise
 */
VEC_STATIC bool VEC_QSORT (struct VEC_VEC * self, int compar (const void *, const void *))
{
    if (self == NULL || self->ptr == NULL || compar == NULL)
        return false;
    qsort(self->ptr, self->length, sizeof(VEC_DATA_TYPE), compar);
    return true;
}

/**=========================================================
 * @brief Apply @a f on every element of @a self in the range
 *        [@a from, @a to[
 * @param self The vector
 * @param f The function to apply on every element
 * @param from The start index
 * @param to The end index (not including element at this index)
 * @returns `false` if @a self is not a valid vector, @a f is NULL,
 *          @a from is larger than or equal to @a to, or @a to is out of bounds,
 *          `true` otherwise
 */
VEC_STATIC bool VEC_MAP_RANGE (struct VEC_VEC * self, VEC_DATA_TYPE f (VEC_DATA_TYPE), size_t from, size_t to)
{
    if (self == NULL
    || self->ptr == NULL
    || f == NULL
    || from >= to
    || to > self->length)
        return false;

    for (size_t i = from; i < to; i++)
        self->ptr[i] = f(self->ptr[i]);

    return true;
}

/**=========================================================
 * @brief Apply @a f to every element of @a self
 * @param self The vector
 * @param f The function to apply on every element
 * @returns `false` if @a self is not a valid vector or @a f is NULL,
 *          `true` otherwise
 */
VEC_STATIC bool VEC_MAP (struct VEC_VEC * self, VEC_DATA_TYPE f (VEC_DATA_TYPE))
{
    return (self != NULL) ?
        VEC_MAP_RANGE(self, f, 0, self->length) :
        false;
}

/**=========================================================
 * @brief Start iterating over @a self. If @a self is already iterating,
 *        do nothing
 * @param self The vector
 * @returns `false` if @a self is not a valid vector or it is already
 *          iterating, `true` otherwise
 */
VEC_STATIC bool VEC_ITER (struct VEC_VEC * self)
{
    if (self == NULL
    || self->iterating
    || self->ptr == NULL
    || self->length == 0)
        return false;

    self->idx = (self->reverse) ?
        self->length - 1 :
        0;

    self->iterating = true;

    return true;
}

/**=========================================================
 * @brief Stop the iterator over @a self
 * @param self The vector
 * @returns `false` if @a self is not a valid vector, `true` otherwise
 */
VEC_STATIC inline bool VEC_ITER_END (struct VEC_VEC * self)
{
    return (self != NULL) && !(self->iterating = false);
}

/**=========================================================
 * @brief Check if @a self is iterating
 * @param self The vector
 * @returns `true` if @a self is iterating, `false` otherwise
 */
VEC_STATIC inline bool VEC_ITERING (const struct VEC_VEC * self)
{
    return (self != NULL) && self->iterating;
}

/**=========================================================
 * @brief Get the index of the iterator
 * @param self The vector
 * @returns The current index of the iterator
 */
VEC_STATIC inline size_t VEC_ITER_IDX (struct VEC_VEC * self)
{
    assert(self != NULL);
    assert(self->idx < self->length);
    return self->idx;
}

/**=========================================================
 * @brief Move the iterator to the next index
 * @param self The vector
 * @returns `true` if still iterating, `false` otherwise
 */
VEC_STATIC bool VEC_ITER_NEXT (struct VEC_VEC * self)
{
    if (self == NULL || self->idx >= self->length || !self->iterating)
        return false;

    bool over = self->idx == ((!self->reverse) ?
            self->length - 1 :
            0);

    if (over)
        VEC_ITER_END(self);
    else
        if (self->reverse)
            self->idx--;
        else
            self->idx++;

    return self->iterating;
}

/**=========================================================
 * @brief Set the `reverse` flag, i.e., if the iterator should
 *        move from beggining to end or end to begginning
 * @param self The vector
 * @returns `false` if @a self is iterating or it is not a valid vector,
 *          `true` otherwise
 */
VEC_STATIC bool VEC_ITER_REV (struct VEC_VEC * self, bool rev)
{
    if (self == NULL || self->iterating)
        return false;
    self->reverse = rev;
    return true;
}

#endif /* VEC_IMPLEMENTATION */

/*==========================================================
 * Clean up
 *=========================================================*/

/*
 * Functions
 */
#undef VEC_APPEND
#undef VEC_AS_MUT_SLICE
#undef VEC_AS_SLICE
#undef VEC_BSEARCH
#undef VEC_CAPACITY
#undef VEC_ELEM
#undef VEC_FILTER
#undef VEC_FIND
#undef VEC_FREE
#undef VEC_FREE_RANGE
#undef VEC_FROM_RAW_PARTS
#undef VEC_GET_NTH
#undef VEC_INSERT
#undef VEC_IS_EMPTY
#undef VEC_ITER
#undef VEC_ITERING
#undef VEC_ITER_END
#undef VEC_ITER_IDX
#undef VEC_ITER_NEXT
#undef VEC_ITER_REV
#undef VEC_LEN
#undef VEC_MAP
#undef VEC_MAP_RANGE
#undef VEC_NEW
#undef VEC_POP
#undef VEC_PUSH
#undef VEC_QSORT
#undef VEC_REMOVE
#undef VEC_RESERVE
#undef VEC_SET_LEN
#undef VEC_SET_NTH
#undef VEC_SHRINK_TO_FIT
#undef VEC_SPLIT_OFF
#undef VEC_SWAP_REMOVE
#undef VEC_TRUNCATE
#undef VEC_WITH_CAPACITY
#undef _VEC_CHANGE_CAPACITY
#undef _VEC_DECREASE_CAPACITY
#undef _VEC_INCREASE_CAPACITY

/*
 * Other
 */
#undef VEC_CONCAT
#undef VEC_DATA_TYPE
#undef VEC_DATA_TYPE_EQ
#undef VEC_DTOR
#undef VEC_MAKE_STR
#undef VEC_MAKE_STR1
#undef VEC_PREFIX
#undef VEC_STATIC
#undef VEC_VEC

/*==========================================================
 * License
 *==========================================================
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */
