#ifndef MBS_H
#define MBS_H

/* mbs - locale dependent multi-byte string functions
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
#include <wchar.h>

LIBMBA_API int mbslen(const char *src);
LIBMBA_API int mbsnlen(const char *src, size_t sn, int cn);

LIBMBA_API size_t mbssize(const char *src);
LIBMBA_API size_t mbsnsize(const char *src, size_t sn, int cn);

LIBMBA_API char *mbsdup(const char *src);
LIBMBA_API char *mbsndup(const char *src, size_t n, int cn);

LIBMBA_API char *mbsoff(char *src, int off);
LIBMBA_API char *mbsnoff(char *src, int off, size_t sn);

LIBMBA_API char *mbschr(char *src, wchar_t wc);
LIBMBA_API char *mbsnchr(char *src, size_t sn, int cn, wchar_t wc);

LIBMBA_API int mbswidth(const char *src, size_t sn, int wn);

LIBMBA_API char *mbssub(char *src, size_t sn, int wn);

#ifdef __cplusplus
}
#endif

#endif /* MBS_H */
