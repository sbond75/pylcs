#ifndef MSGNO_H
#define MSGNO_H

/* msgno - managing error codes and associated messages across
 * separate C libraries
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

#if defined(__GNUC__)
#if defined(MSGNO)

#include <stdio.h>

#if defined(__GNUC__) && (__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 95))

#define MSG(fmt, args...) _msgno_printf("%s:%u:%s: " fmt, \
		__FILE__, __LINE__, __FUNCTION__, ## args)
#define MNO(msgno) _msgno_printf("%s:%u:%s: %s", \
		__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno))
#define MNF(msgno, fmt, args...) _msgno_printf("%s:%u:%s: %s" fmt, \
		__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno), ## args)

#define PMSG(fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
		"%s:%u:%s: " fmt "\n", \
		__FILE__, __LINE__, __FUNCTION__, ## args))
#define PMNO(msgno) (_msgno_buf_idx = sprintf(_msgno_buf, \
		"%s:%u:%s: %s\n", \
		__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno)))
#define PMNF(msgno, fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
		"%s:%u:%s: %s" fmt "\n", \
		__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno), ## args))

#define AMSG(fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
		"  %s:%u:%s: " fmt "\n", \
		__FILE__, __LINE__, __FUNCTION__, ## args))
#define AMNO(msgno) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
		"  %s:%u:%s: %s\n", \
		__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno)))
#define AMNF(msgno, fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
		"  %s:%u:%s: %s" fmt "\n", \
		__FILE__, __LINE__, __FUNCTION__, msgno_msg(msgno), ## args))

#else

#define MSG(fmt, args...) _msgno_printf("%s:%u: " fmt "\n", \
		__FILE__, __LINE__ , ## args)
#define MNO(msgno) _msgno_printf("%s:%u: %s\n", \
		__FILE__, __LINE__, msgno_msg(msgno))
#define MNF(msgno, fmt, args...) _msgno_printf("%s:%u: %s" fmt "\n", \
		__FILE__, __LINE__, msgno_msg(msgno) , ## args)

#define PMSG(fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
		"%s:%u: " fmt "\n", __FILE__, __LINE__ , ## args))
#define PMNO(msgno) (_msgno_buf_idx = sprintf(_msgno_buf, \
		"%s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno)))
#define PMNF(msgno, fmt, args...) (_msgno_buf_idx = sprintf(_msgno_buf, \
		"%s:%u: %s" fmt "\n", __FILE__, __LINE__, msgno_msg(msgno) , ## args))

#define AMSG(fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
		"  %s:%u: "fmt"\n", __FILE__, __LINE__ , ## args))
#define AMNO(msgno) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
		"  %s:%u: %s\n", __FILE__, __LINE__, msgno_msg(msgno)))
#define AMNF(msgno, fmt, args...) (_msgno_buf_idx += sprintf(_msgno_buf + _msgno_buf_idx, \
		"  %s:%u: %s" fmt "\n", __FILE__, __LINE__, msgno_msg(msgno) , ## args))

#endif
#else

#define MSG(fmt, args...)
#define MNO(msgno)
#define MNF(msgno, fmt, args...)
#define PMSG(fmt, args...)
#define PMNO(msgno)
#define PMNF(msgno, fmt, args...)
#define AMSG(fmt, args...)
#define AMNO(msgno)
#define AMNF(msgno, fmt, args...)

#endif
#else
#undef MSG
#if defined(MSGNO)

#define MSG msgno_hdlr
#define MNO msgno_hdlr_mno
#define MNF msgno_hdlr_mnf
#define PMSG msgno_hdlr
#define PMNO msgno_hdlr_mno
#define PMNF msgno_hdlr_mnf
#define AMSG msgno_hdlr
#define AMNO msgno_hdlr_mno
#define AMNF msgno_hdlr_mnf

#else

#define MSG msgno_noop_msg
#define MNO msgno_noop_mno
#define MNF msgno_noop_mnf
#define PMSG msgno_noop_msg
#define PMNO msgno_noop_mno
#define PMNF msgno_noop_mnf
#define AMSG msgno_noop_msg
#define AMNO msgno_noop_mno
#define AMNF msgno_noop_mnf

#endif
#endif

#define NULL_POINTER_ERR _builtin_codes[0].msgno

struct msgno_entry {
	int msgno;
	const char *msg;
};

LIBMBA_API struct msgno_entry _builtin_codes[];
LIBMBA_API char _msgno_buf[];
LIBMBA_API unsigned int _msgno_buf_idx;
LIBMBA_API int (*msgno_hdlr)(const char *fmt, ...);
LIBMBA_API void _msgno_printf(const char *fmt, ...);

LIBMBA_API int msgno_add_codes(struct msgno_entry *lst);
LIBMBA_API const char *msgno_msg(int msgno);
LIBMBA_API int msgno_hdlr_stderr(const char *fmt, ...);

LIBMBA_API int msgno_hdlr_mno(int msgno);
LIBMBA_API int msgno_hdlr_mnf(int msgno, const char *fmt, ...);
LIBMBA_API int msgno_noop_msg(const char *fmt, ...);
LIBMBA_API int msgno_noop_mno(int msgno);
LIBMBA_API int msgno_noop_mnf(int msgno, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* MSGNO_H */

