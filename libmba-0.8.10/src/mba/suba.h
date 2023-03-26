#ifndef SUBA_H
#define SUBA_H

/* suba - sub-allocate memory from larger chunk of memory
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

#include <stddef.h>
#include <mba/allocator.h>

#define _R(a,p) ((ref_t)((p) ? (char *)(p) - (char *)(a) : 0))
#define _A(a,r) ((void *)((r) ? (char *)(a) + (r) : NULL))

LIBMBA_API struct allocator *suba_init(void *mem, size_t size, int rst, size_t mincell);
LIBMBA_API void *suba_alloc(struct allocator *suba, size_t size, int zero);
LIBMBA_API void *suba_realloc(struct allocator *suba, void *ptr, size_t size);
LIBMBA_API int suba_free(void *suba, void *ptr);
LIBMBA_API void *suba_addr(const struct allocator *suba, const ref_t ref);
LIBMBA_API ref_t suba_ref(const struct allocator *suba, const void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* SUBA_H */

