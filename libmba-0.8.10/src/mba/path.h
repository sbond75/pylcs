#ifndef PATH_H
#define PATH_H

/* path - manipulate file path names
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

LIBMBA_API int path_canon(const unsigned char *src, const unsigned char *slim,
		unsigned char *dst, unsigned char *dlim,
		int srcsep, int dstsep);

#ifdef __cplusplus
}
#endif

#endif /* PATH_H */
