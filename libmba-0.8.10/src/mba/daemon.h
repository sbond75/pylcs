#ifndef DAEMON_H
#define DAEMON_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

extern FILE *logfp;

pid_t daemonize(mode_t mask,
		const char *runpath,
		const char *pidpath,
		const char *lockpath,
		const char *logpath);
int daemonlog(const char *fmt, ...);

#endif /* DAEMON_H */
