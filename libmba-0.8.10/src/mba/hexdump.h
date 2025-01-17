#ifndef HEXDUMP_H
#define HEXDUMP_H

/* hexdump - print hexdump of memory to stream
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

LIBMBA_API void hexdump(FILE *stream, const void *src, size_t n, size_t width);
LIBMBA_API int shexdump(const void *src, size_t len, size_t width, char *dst, char *dlim);
LIBMBA_API const char *mbstoax(const char *src, size_t sn, int wn);

#ifdef __cplusplus
}
#endif

#endif /* HEXDUMP_H */

