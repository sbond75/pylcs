/* msgno - managing error codes and associated messages across
 * separate C libraries
 * Copyright (c) 2001 Michael B. Allen <mba2000 ioplex.com>
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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "mba/msgno.h"
#include "defines.h"

#ifndef MSGNO_NUM_LISTS
#define MSGNO_NUM_LISTS 16
#endif
#ifndef MSGNO_BUFSIZ
#define MSGNO_BUFSIZ 1024
#endif

struct msgno_entry _builtin_codes[2] = {
	{ (1 << 16), "A parameter was NULL" },
	{ 0, NULL }
};

static struct tbl_entry {
	struct msgno_entry *list;
	unsigned int num_msgs;
} list_tbl[MSGNO_NUM_LISTS] = {
	{ _builtin_codes, 1 }
};

static unsigned int next_tbl_idx = 1;

char _msgno_buf[MSGNO_BUFSIZ];
unsigned int _msgno_buf_idx;

int
msgno_add_codes(struct msgno_entry *list)
{
	struct tbl_entry *te;
	int next_msgno = 0, hi_bits;

	if (list == NULL || list->msg == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (next_tbl_idx == MSGNO_NUM_LISTS) {
		errno = ERANGE;
		return -1;
	}

	for (te = list_tbl + 1; te->list; te++) {
		if (te->list == list) {
			return 0; /* already in list_tbl */
		}
	}

	hi_bits = (next_tbl_idx + 1) << 16;
	te->list = list;
	while (list->msg) {
		if ((list->msgno & 0xFFFF0000)) {
			te->list = NULL;
			errno = ERANGE;
			return -1;
		}
		if (list->msgno == 0) {
			list->msgno = hi_bits | next_msgno++;
		} else if (list->msgno >= next_msgno) {
			next_msgno = list->msgno + 1;
			list->msgno = hi_bits | list->msgno;
		} else {
			te->list = NULL;
			errno = ERANGE;
			return -1;
		}
		te->num_msgs++;
		list++;
	}
	next_tbl_idx++;

	return 0;
}

const char *
msgno_msg(int msgno)
{
	struct tbl_entry *te;
	unsigned int i;

	i = msgno >> 16;
	if (i == 0) {
		return strerror(msgno);
	} else if (i >= MSGNO_NUM_LISTS ||
					(te = list_tbl + (i - 1)) == NULL) {
		return "No such msgno list";
	}

	for (i = 0; i < te->num_msgs; i++) {
		if (te->list[i].msgno == msgno) {
			return te->list[i].msg;
		}
	}

	return "No such message in msgno list";
}

int
msgno_hdlr_stderr(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputc('\n', stderr);
	fflush(stderr);
	return 0;
}
int
msgno_hdlr_mno(int msgno)
{
	msgno_hdlr("%s", msgno_msg(msgno));
	return 0;
}
int
msgno_hdlr_mnf(int msgno, const char *fmt, ...)
{
	char buf[4096];
	int n;
	va_list ap;
	va_start(ap, fmt);

	n = sprintf(buf, "%s: ", msgno_msg(msgno));
	n += vsprintf(buf + n, fmt, ap);
	buf[n] = '\0';
	msgno_hdlr("%s", buf);

	va_end(ap);
	return 0;
}

#if HAVE_VARMACRO < 1
int
msgno_noop_msg(const char *fmt, ...)
{
	return 0;
}
int
msgno_noop_mno(int msgno)
{
	return 0;
}
int
msgno_noop_mnf(int msgno, const char *fmt, ...)
{
	return 0;
}
#endif


int (*msgno_hdlr)(const char *fmt, ...) = msgno_hdlr_stderr;

void
_msgno_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	if (*_msgno_buf) {
		_msgno_buf[_msgno_buf_idx++] = ' ';
		_msgno_buf[_msgno_buf_idx++] = ' ';
	}
	vsprintf(_msgno_buf + _msgno_buf_idx, fmt, ap);
	msgno_hdlr("%s", _msgno_buf);
	*_msgno_buf = '\0';
	_msgno_buf_idx = 0;
	va_end(ap);
}

