#ifndef _IFJMP_H
#define _IFJMP_H

/*
 * # Examples
 *
 * ```c
 * int side_effects (int x)
 * {
 * # if 0
 *     // Bad
 *     ifjmp(++x > 0, out); // You probably don't want this
 * # else
 *     // Good
 *     x++;
 *     ifjmp(x > 0, out);
 * # endif
 *     x++;
 * out:
 *     return x;
 * }
 *
 * // Non-void function, no dynamic memory
 * int non_void_no_dynamic_mem (int x)
 * {
 * # if 0
 *     // `y` will be used but is not defined
 *     ifjmp(x == 42, out);
 *     int y = x + 2;
 * # else
 *     // `y` was defined and initialized before `ifjmp()`
 *     int y = x + 2;
 *     ifjmp(x == 42, out);
 * # endif
 *
 * out:
 *     return y;
 * }
 *
 * // Non-void function, dynamic memory
 * void * example2 (size_t size)
 * {
 * # if 0
 *     void * ret = malloc(size);
 * # else
 *     // We could also check `size` and abort if `size == 0`
 *     void * ret = NULL;
 *     ifjmp(size == 0, out);
 *     ret = malloc(size);
 * # endif
 *
 *     // If `ret == NULL` there's nothing to do, get out
 *     ifjmp(ret == NULL, out);
 *
 *     // Getting this far means `ret != NULL`
 *
 *     // If `ret` does not pass some funcion `something()`
 *     // `NULL` should be returned instead
 *     int cond = something(ret);
 *
 *     // If `ret` does not pass this condition, it must
 *     // be `free()`d. Going `out` instead of `ko` would
 *     // lead to a memory leak
 *     ifjmp(cond, ko);
 *
 *     // Getting to this line means `ret` passed the
 *     // condition and so it is good to be returned
 *
 * out:
 *     return ret;
 *
 * ko:
 *     // Do some cleanup here
 *     free(ret);
 *
 * # if 1
 *     ret = NULL;
 *     goto out;
 * # else // Either one of these is fine
 *     return NULL;
 * # endif
 * }
 *
 * // void function, dynamic memory involved
 * void void_dynamic_mem (size_t size)
 * {
 *     // If there's nothing to do, get out
 *     ifjmp(!should_do_something, out);
 *
 *     void * mem = malloc(size);
 *
 *     // If `mem == NULL` there's nothing to do, get out
 *     ifjmp(mem == NULL, out);
 *
 *     do_something(mem);
 *
 *     ifjmp(!should_do_something_else, cleanup);
 *
 *     do_something_else(mem);
 *
 * cleanup:
 *     free(mem); // `mem != NULL`
 *
 * out:
 *     return;
 * }
 * ```
 */

/**
 * @brief Jump to a label if a condition verifies.
 * @param COND The condition to test
 * @param LBL The label to jump to
 */
#define ifjmp(COND, LBL) if (COND) goto LBL

#endif /* _IFJMP_H */
