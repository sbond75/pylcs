/* sock.h - a network socket interface
 */

#ifndef SOCK_H
#define SOCK_H

#include <mba/msgno.h>

extern int sockerr;
extern struct msgno_entry sock_codes[];

#define SOCK_BIND_ERR    sock_codes[0].msgno
#define SOCK_UKNOWN_HOST sock_codes[1].msgno

struct sock {
} s;

struct sock *sock_open(const char *host, int port);
int sock_write(const char *data);

#endif /* SOCK_H */
