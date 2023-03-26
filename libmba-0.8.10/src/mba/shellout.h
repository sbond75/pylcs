#ifndef SHELLOUT_H
#define SHELLOUT_H

/* shellout - execute programs in a pty shell programmatically
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <termios.h>

#define SHO_FLAGS_INTERACT 0x0001
#define SHO_FLAGS_ISATTY   0x0100

struct sho {
	int flags;
	pid_t pid;
	int ptym;
	struct termios t0;
};

struct sho *sho_open(const char *sh, const char *ps1, int flags);
int sho_close(struct sho *sh);
int sho_expect(struct sho *sh, const char *pv[], int pn, char *dst, size_t dn, int timeout);
int sho_loop(struct sho *sh, const char *pv[], int pn, int timeout);

ssize_t readn(int fd, void *dst, size_t n);
ssize_t writen(int fd, const void *src, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* SHELLOUT_H */
