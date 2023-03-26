#ifndef TIME_H
#define TIME_H

/* time - supplimentary time functions
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

#if defined(__sparc__)
  #include <sys/inttypes.h>
#elif defined(_WIN32)
  #define MILLISECONDS_BETWEEN_1970_AND_1601 11644473600000Ui64
  typedef unsigned __int64 uint64_t;
#else
  #include <stdint.h>
#endif

LIBMBA_API uint64_t time_current_millis(void);

#ifdef __cplusplus
}
#endif

#endif /* TIME_H */
