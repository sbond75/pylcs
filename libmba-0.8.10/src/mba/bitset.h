#ifndef BITSET_H
#define BITSET_H

/* bitset - a set of bits
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIBMBA_API
#ifdef WIN32
# ifdef LIBMBA_EXPORTS
#  define LIBMBA_API  __declspec(dllexport)
# else /* LIBMBA_EXPORTS */
#  define LIBMBA_API  __declspec(dllimport)
# endif /* LIBMBA_EXPORTS */
#else /* WIN32 */
# define LIBMBA_API extern
#endif /* WIN32 */
#endif /* LIBMBA_API */

#include <limits.h>
#include <mba/iterator.h>

#define bitset_elem(ptr,bit) ((unsigned char *)(ptr))[(bit)/CHAR_BIT]
#define bitset_mask(ptr,bit) (1U << ((bit) % CHAR_BIT))

#define bitset_isset(ptr,bit) ((bitset_elem(ptr,bit) & bitset_mask(ptr,bit)) != 0)
#define bitset_set(ptr,bit) (bitset_elem(ptr,bit) |= bitset_mask(ptr,bit))
#define bitset_unset(ptr,bit) (bitset_elem(ptr,bit) &= ~bitset_mask(ptr,bit))
#define bitset_toggle(ptr,bit) (bitset_elem(ptr,bit) ^= bitset_mask(ptr,bit))

LIBMBA_API int bitset_find_first(void *ptr, void *plim, int val);
LIBMBA_API void bitset_iterate(iter_t *iter);
LIBMBA_API int bitset_next(void *ptr, void *plim, iter_t *iter);

#ifdef __cplusplus
}
#endif

#endif /* BITSET_H */
