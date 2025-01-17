/* svsem - POSIX-like semaphores implemented using SysV semaphores
 * Copyright (c) 2003 Michael B. Allen <mba2000 ioplex.com>
 *
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "mba/svsem.h"
#include "mba/pool.h"
#include "mba/varray.h"
#include "mba/misc.h"
#include "mba/msgno.h"
#include "defines.h"

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#include <sys/sem.h>
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;                  /* value for SETVAL */
	struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
	unsigned short *array;    /* array for GETALL, SETALL */
	/* Linux specific part: */
	struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

#define MAGIC 0xAD800000
#define MAX_TRIES 10
#define SEM_ISVALID(s) ((s) && ((s)->flags & 0xFFF00000) == MAGIC)

static int
semid_get(const char *name, int nsems, int oflag, mode_t mode, unsigned int value, struct allocator *al)
{
	key_t key;
	int semid, max;

	if ((key = ftok(name, 1)) == (key_t)-1) {
		PMNF(errno, ": %s", name);
		return -1;
	}
	if ((oflag & O_CREAT) == 0) {
		if ((semid = semget(key, 0, 0)) == -1) {
			PMNF(errno, ": %s", name);
			return -1;
		}
		return semid;
	}

	/* This outer try-loop ensures that we know if the semaphore was created
	 * as opposed to just opened even if it was created after trying to open
	 * without IPC_EXCL or deleted after trying to open with IPC_EXCL.
	 */
	max = MAX_TRIES;
	for (max = MAX_TRIES; max; max--) {
		union semun arg;

		if ((oflag & O_EXCL) == 0) {
                                            /* fail if it exists */
			if ((semid = semget(key, nsems, 0)) != -1) {
				struct semid_ds buf;
	
				/* This inner try-loop ensures that the semaphore is initialized before
				 * we return even if the semaphore has been created with semget but not
				 * yet initialized with semctl.
				 */
				arg.buf = &buf;
				max = MAX_TRIES;
				for (max = MAX_TRIES; max; max--) {
					if (semctl(semid, 0, IPC_STAT, arg) == -1) {
						PMNF(errno, ": %s", name);
						return -1;
					}
					if (buf.sem_otime != 0) {
						return semid;
					}
					sleep(1);
				}
	
				errno = ETIME;
				PMNF(errno, ": %s", name);
				return -1;
			} else if (errno != ENOENT) {
				PMNF(errno, ": %s", name);
				return -1;
			}
		}
                                    /* fail if it does not exist */
		if ((semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | (mode & 0777))) != -1) {
			struct sembuf initop;

			/* we created the semaphore, perform method to subvert inialization race
			 * as described in Stevens' UNPv2 p274
			 */

			if (nsems > 1) {
				int i;

				if ((arg.array = allocator_alloc(al, nsems * sizeof(unsigned short), 0)) == NULL) {
					AMSG("");
					semctl(semid, 0, IPC_RMID);
					return -1;
				}
				arg.array[0] = 0; /* leave the first one 0 to be set with semop */
				for (i = 1; i < nsems; i++) {
					arg.array[i] = value;
				}
				if (semctl(semid, 0, SETALL, arg) == -1) {
					PMNO(errno);
					allocator_free(al, arg.array);
					semctl(semid, 0, IPC_RMID);
					return -1;
				}
				allocator_free(al, arg.array);
			} else {
				arg.val = 0;
				if (semctl(semid, 0, SETVAL, arg) == -1) {
					PMNO(errno);
					semctl(semid, 0, IPC_RMID);
					return -1;
				}
			}

			initop.sem_num = 0;
			initop.sem_op = value;
			initop.sem_flg = 0;
			if (semop(semid, &initop, 1) == -1) {
				semctl(semid, 0, IPC_RMID);
				return -1;
			}

			return semid;
		} else if ((oflag & O_EXCL) || errno != EEXIST) {
			PMNF(errno, ": %s", name);
			return -1;
		}
	}

	errno = EACCES;
	PMNF(errno, ": %s", name);
	return -1;
}
static void *
_svs_new(void *context, size_t size, int flags)
{
	struct _svs_data *sd = context;
	svsem_t *sem;

				/* size is really semaphore number */
	if ((sem = varray_get(&sd->sems, size)) == NULL) {
		AMSG("");
		return NULL;
	}
	sem->id = sd->id;
	sem->num = size;
	sem->flags = MAGIC | (flags & 0x000FFFFFF);

                             /* is this necessary? */
	if (svsem_setvalue(sem, sd->val) == -1) {
		AMSG("");
		sem->flags = 0;
		return NULL;
	}

	return sem;
}
static int
_svs_del(void *context, void *object)
{
	svsem_t *sem = object;

	sem->id = 0;
	sem->flags = 0;
	context = NULL;

	return 0;
}
static int
_svs_rst(void *context, void *object)
{
	struct _svs_data *sd = context;
	svsem_t *sem = object;

	if (svsem_setvalue(sem, sd->val) == -1) {
		AMSG("");
		return -1;
	}

	return 0;
}
int
svsem_pool_destroy(struct pool *p)
{
	int ret = 0;

	if (p) {
		struct _svs_data *sd = p->context;

		ret += varray_deinit(&sd->sems);
		ret += pool_destroy(p);
		ret += semctl(sd->id, 0, IPC_RMID);
		unlink(sd->name);
		ret += allocator_free(p->al, sd);
	}

	return ret;
}
int
svsem_pool_create(struct pool *p,
		unsigned int max_size,
		unsigned int value,
		int undo,
		struct allocator *al)
{
	struct _svs_data *sd;
	int fd, nsems = max_size;

	memset(p, 0, sizeof *p);

	if ((sd = allocator_alloc(al, sizeof *sd, 0)) == NULL) {
		AMSG("");
		return -1;
	}

	strcpy(sd->name, "/tmp/svsemXXXXXX");
	if ((fd = mkstemp(sd->name)) == -1) {
		PMNO(errno);
		allocator_free(al, sd);
		return -1;
	}
	close(fd);

	if ((sd->id = semid_get(sd->name, nsems, O_CREAT, 0666, value, al)) == -1) {
		AMSG("");
		allocator_free(al, sd);
		unlink(sd->name);
		return -1;
	}
	unlink(sd->name);

	if (pool_create(p, max_size, _svs_new, _svs_del, _svs_rst, sd, -1, undo ? SEM_UNDO : 0, al) == -1) {
		AMSG("");
		semctl(sd->id, 0, IPC_RMID);
		allocator_free(al, sd);
		return -1;
	}

	sd->val = value;
	if (varray_init(&sd->sems, offsetof(svsem_t, name), al) == -1) {
		AMSG("");
		svsem_pool_destroy(p);
		return -1;
	}

	return 0;
}
int
svsem_open(svsem_t *sem, const char *path, int oflag, ...)
{
	mode_t mode = 0;
	int val = 0;

	if ((oflag & O_CREAT)) {
		va_list ap;
		int fd, cre;

		va_start(ap, oflag);
		mode = va_arg(ap, mode_t);
		val = va_arg(ap, unsigned int);
		va_end(ap);

		if ((fd = copen(path, oflag, mode, &cre)) == -1) {
			PMNF(errno, ": %s", path);
			return -1;
		}
                      /* should we really assert the mode? */
		if (cre && fchmod(fd, mode) == -1) {
			PMNF(errno, ": %s", path);
			close(fd);
			return -1;
		}
		close(fd);
	}

	if ((sem->id = semid_get(path, 1, oflag, mode, val, NULL)) == -1) {
		AMSG("");
		return -1;
	}
	sem->num = 0;
	sem->flags = MAGIC | (oflag & 0x000FFFFFF);
                         /* how do I know this mask is ok? */
	return 0;
}
int
svsem_close(svsem_t *sem)
{
	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	sem->id = sem->flags = 0;

	return 0;
}
int
svsem_remove(svsem_t *sem)
{
	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	if (semctl(sem->id, 0, IPC_RMID) == -1) {
		PMNO(errno);
		return -1;
	}
	sem->id = sem->flags = 0;

	return 0;
}
int
svsem_create(svsem_t *sem, int value, int undo)
{
	int fd;
			/* do not change this path without also adjusting
			 * the size if svsem_t's name member */
	strcpy(sem->name, "/tmp/svsemXXXXXX");
	if ((fd = mkstemp(sem->name)) == -1) {
		PMNO(errno);
		return -1;
	}
	close(fd);

	if ((sem->id = semid_get(sem->name, 1, O_CREAT | O_EXCL, 0600, value, NULL)) == -1) {
		AMSG("");
		return -1;
	}
	sem->num = 0;
	sem->flags = MAGIC | (undo ? SEM_UNDO : 0);

	return 0;
}
int
svsem_destroy(svsem_t *sem)
{
	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	if (semctl(sem->id, 0, IPC_RMID) == -1 && errno != EINVAL) {
		PMNO(errno);
		return -1;
	}
	sem->id = sem->flags = 0;
	unlink(sem->name);

	return 0;
}
int
svsem_wait(svsem_t *sem)
{
	struct sembuf wait;

	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	wait.sem_num = sem->num;
	wait.sem_op = -1;
	wait.sem_flg = sem->flags & SEM_UNDO;

	if (semop(sem->id, &wait, 1) == -1) {
		PMNF(errno, ": %d:%d", sem->id, sem->num);
		return -1;
	}

	return 0;
}
int
svsem_trywait(svsem_t *sem)
{
	struct sembuf wait;

	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	wait.sem_num = sem->num;
	wait.sem_op = -1;
	wait.sem_flg = (sem->flags & SEM_UNDO) | IPC_NOWAIT;

	if (semop(sem->id, &wait, 1) == -1) {
		PMNF(errno, ": %d:%d", sem->id, sem->num);
		return -1;
	}

	return 0;
}
int
svsem_post(svsem_t *sem)
{
	struct sembuf post;

	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	post.sem_num = sem->num;
	post.sem_op = 1;
	post.sem_flg = sem->flags & SEM_UNDO;

	if (semop(sem->id, &post, 1) == -1) {
		PMNF(errno, ": %d:%d", sem->id, sem->num);
		return -1;
	}

	return 0;
}
int
svsem_post_multiple(svsem_t *sem, int count)
{
	int ret = 0;
	struct sembuf post;

	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	post.sem_num = sem->num;
	post.sem_op = 1;
	post.sem_flg = sem->flags & SEM_UNDO;

	while (count--) {
		ret += semop(sem->id, &post, 1);
	}

	if (ret) {
		PMNF(errno, ": %d:%d", sem->id, sem->num);
		return -1;
	}

	return 0;
}
int
svsem_getvalue(svsem_t *sem, int *value)
{
	int v;

	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	if ((v = semctl(sem->id, sem->num, GETVAL, 0)) == -1) {
		PMNO(errno);
		return -1;
	}
	*value = v;

	return 0;
}
int
svsem_setvalue(svsem_t *sem, int value)
{
	union semun arg;

	if (!SEM_ISVALID(sem)) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	arg.val = value;
	if (semctl(sem->id, sem->num, SETVAL, arg) == -1) {
		PMNO(errno);
		return -1;
	}

	return 0;
}
