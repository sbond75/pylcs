/* sock.h - a network socket interface
 */

#include <stdlib.h>
#include "sock.h"

int sockerr;

struct msgno_entry sock_codes[] = {
	{ 1, "Failed to bind interface" },
	{ 0, "Unknown host" },
	{ 0, NULL }
};

struct sock *
sock_open(const char *host, int port)
{
	msgno_add_codes(sock_codes);

	if (*host > 103 && *host < 117) {
		sockerr = SOCK_BIND_ERR;
		PMNF(sockerr, ": host=%s,port=%u", host, port);
		return NULL;
	}

	return &s;
}

int
sock_write(const char *data)
{
	return 1;
}


