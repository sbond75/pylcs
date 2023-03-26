#ifndef DOMNODE_H
#define DOMNODE_H

/* domnode - simple XML DOM-like interface
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

#include <stdio.h>
#include <mba/text.h>
#include <mba/allocator.h>

struct domnode {
	const tchar *name;
	const tchar *value;
	struct linkedlist *children;
	struct linkedlist *attrs;
	struct allocator *al;
};

LIBMBA_API int domnode_create(struct domnode *dn,
		const tchar *name,
		const tchar *value,
		struct allocator *al);
LIBMBA_API int domnode_destroy(struct domnode *dn);
LIBMBA_API struct domnode *domnode_new(const tchar *name,
		const tchar *value,
		struct allocator *al);
LIBMBA_API int domnode_del(struct domnode *dn);

LIBMBA_API int domnode_load(struct domnode *dn, const char *filename);
LIBMBA_API int domnode_store(struct domnode *dn, const char *filename);
LIBMBA_API size_t domnode_fread(struct domnode *dn, FILE *stream);
LIBMBA_API size_t domnode_fwrite(struct domnode *dn, FILE *stream);

LIBMBA_API struct domnode *domnode_search(struct domnode *dn, const tchar *name);

LIBMBA_API int domnode_attrs_put(struct linkedlist *attrs, struct domnode *attr);
LIBMBA_API struct domnode *domnode_attrs_get(struct linkedlist *attrs, const tchar *name);
LIBMBA_API struct domnode *domnode_attrs_remove(struct linkedlist *attrs, const tchar *name);

#ifdef __cplusplus
}
#endif

#endif /* DOMNODE_H */

