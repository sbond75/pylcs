#ifndef VARRAY_H
#define VARRAY_H

/* varray - a variable sized array
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
#include <mba/iterator.h>

#ifndef VARRAY_INIT_SIZE
#define VARRAY_INIT_SIZE 6
#endif

struct varray {
	size_t size;                                          /* element size */
	ptrdiff_t al;  /* relative offset of this object to allocator of bins */
	ref_t bins[16];                                 /* 0 to 2^21 elements */
};

LIBMBA_API int varray_init(struct varray *va, size_t membsize, struct allocator *al);
LIBMBA_API int varray_reinit(struct varray *va, struct allocator *al);
LIBMBA_API int varray_deinit(struct varray *va);
LIBMBA_API struct varray *varray_new(size_t membsize, struct allocator *al);
LIBMBA_API int varray_del(void *va);
LIBMBA_API int varray_release(struct varray *va, unsigned int from);
LIBMBA_API void *varray_get(struct varray *va, unsigned int idx);
LIBMBA_API void varray_iterate(void *va, iter_t *iter);
LIBMBA_API void *varray_next(void *va, iter_t *iter);

#ifdef __cplusplus
}
#endif

#endif /* VARRAY_H */
