/* mbs - locale dependent multi-byte string functions
 * Copyright (c) 2002 Michael B. Allen <mba2000 ioplex.com>
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
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include "mba/msgno.h"
#include "mba/hexdump.h"
#include "mba/mbs.h"
#include "defines.h"

#if !defined(__USE_XOPEN)
int mk_wcwidth(wchar_t ucs);
#define wcwidth mk_wcwidth
#endif

int
mbslen(const char *src)
{
	return mbsnlen(src, -1, -1);
}
int
mbsnlen(const char *src, size_t sn, int cn)
{
	wchar_t ucs;
	int count, w;
	size_t n;
#if HAVE_MBSTATE > 0
	mbstate_t ps;
#endif

	ucs = 1;
	count = 0;

	if (sn > INT_MAX) {
		sn = INT_MAX;
	}
	if (cn < 0) {
		cn = INT_MAX;
	}

#if HAVE_MBSTATE
	memset(&ps, 0, sizeof(ps));
	while (ucs && (n = mbrtowc(&ucs, src, sn, &ps)) != (size_t)-2) {
#else
	while (ucs && sn > 0) {
		n = mbtowc(&ucs, src, sn);
#endif
		if (n == (size_t)-1) {
			PMNO(errno);
			return -1;
		}
		if ((w = wcwidth(ucs)) == -1) {
			w = 1;
		}
		if (w > cn) {
			break;
		}
		cn -= w;
		sn -= n;
		src += n;
		count += w;
	}

	return count;
}

size_t
mbssize(const char *src)
{
	return mbsnsize(src, -1, -1);
}
size_t
mbsnsize(const char *src, size_t sn, int cn)
{
	wchar_t ucs;
	int w;
	size_t tot, n;
#if HAVE_MBSTATE > 0
	mbstate_t ps;
#endif

	tot = n = 0;
	ucs = 1;

	if (sn > INT_MAX) {
		sn = INT_MAX;
	}
	if (cn < 0) {
		cn = INT_MAX;
	}

#if HAVE_MBSTATE
	memset(&ps, 0, sizeof(ps));
	while (ucs && sn > 0 && (n = mbrtowc(&ucs, src, sn, &ps)) != (size_t)-2 && n) {
#else
	while (ucs && sn > 0 && (n = mbtowc(&ucs, src, sn))) {
#endif
		if (n == (size_t)-1) {
			PMNO(errno);
			return -1;
		}
		if ((w = wcwidth(ucs)) == -1) {
			w = 1;
		}
		if (w > cn) {
			break;
		}
		cn -= w;
		sn -= n;
		src += n;
		tot += n;
	}

	return tot;
}

char *
mbsdup(const char *src)
{
	return mbsndup(src, -1, -1);
}
char *
mbsndup(const char *src, size_t sn, int cn)
{
	size_t n;
	char *dst;

	if (src == NULL) {
		errno = EINVAL;
		PMNO(errno);
		return NULL;
	}
	if ((n = mbsnsize(src, sn, cn)) == (size_t)-1) {
		AMSG("");
		return NULL;
	}
	if ((dst = malloc(n + 1)) == NULL) {
		PMNO(errno);
		return NULL;
	}
	memcpy(dst, src, n);
	dst[n] = '\0';

	return dst;
}

char *
mbsoff(char *src, int off)
{
	return mbsnoff(src, off, -1);
}
char *
mbsnoff(char *src, int off, size_t sn)
{
	wchar_t ucs;
	size_t n;
	int w;
#if HAVE_MBSTATE > 0
	mbstate_t ps;
#endif

	if (off == 0) {
		return src;
	}

	if (sn > INT_MAX) {
		sn = 0xFFFF;
	}
	if (off < 0) {
		off = 0xFFFF;
	}

#if HAVE_MBSTATE > 0
	memset(&ps, 0, sizeof(ps));
	while (sn > 0 && (n = mbrtowc(&ucs, src, sn, &ps)) != (size_t)-2) {
#else
	while (sn > 0) {
	 	n = mbtowc(&ucs, src, sn);
#endif
		if (n == (size_t)-1) {
			PMNF(errno, "src=[%s]", mbstoax(src, sn, 1));
			return NULL;
		}
		if (n == 0 || (w = wcwidth(ucs)) != 0) {
			w = 1;
		}
		if (w > off) {
			break;
		}
		if (w) off--;
		sn -= n;
		src += n ? n : 1;
	}

	return src;
}
char *
mbssub(char *src, size_t sn, int wn)
{
	wchar_t wc;
	size_t n;
	int w;
#if HAVE_MBSTATE > 0
	mbstate_t ps;
#endif

	if (wn == 0) {
		return src;
	}

	if (sn > INT_MAX) {
		sn = INT_MAX;
	}
	if (wn < 0) {
		wn = INT_MAX;
	}

#if HAVE_MBSTATE > 0
	memset(&ps, 0, sizeof ps);
	while (sn > 0 && (n = mbrtowc(&wc, src, sn, &ps)) != (size_t)-2) {
#else
	while (sn > 0) {
		n = mbtowc(&wc, src, sn);
#endif
		if (n == (size_t)-1) {
			PMNO(errno);
			return NULL;
		}
		if (n == 0 || (w = wcwidth(wc)) < 0) {
			w = 1;
		}
		if (w > wn) {
			break;
		}
		wn -= w;
		sn -= n;
		src += n;
	}

	return src;
}
char *
mbschr(char *src, wchar_t wc)
{
	return mbsnchr(src, -1, -1, wc);
}
char *
mbsnchr(char *src, size_t sn, int cn, wchar_t wc)
{
	wchar_t ucs;
	size_t n;
#if HAVE_MBSTATE
	mbstate_t ps;
#endif

	if (sn > INT_MAX) {
		sn = INT_MAX;
	}
	if (cn < 0) {
		cn = INT_MAX;
	}

#if HAVE_MBSTATE
	memset(&ps, 0, sizeof(ps));
	while (sn > 0 && cn > 0 &&
				(n = mbrtowc(&ucs, src, sn, &ps)) != (size_t)-2 && n != (size_t)-1) {
#else
	while (sn > 0 && cn > 0 && (n = mbtowc(&ucs, src, sn)) != (size_t)-1) {
#endif
		if (wc == ucs) {
			return src;
		}
		sn -= n;
		src += n ? n : 1;
		if ((n == 0 || wcwidth(ucs) != 0) && --cn == 0) {
			break;
		}
	}

	return NULL;
}

int
mbswidth(const char *src, size_t sn, int wn)
{
	wchar_t wc;
	int w, width;
	size_t n;
#if HAVE_MBSTATE
	mbstate_t ps;
#endif

	wc = 1;
	width = 0;

	if (sn > INT_MAX) {
		sn = INT_MAX;
	}
	if (wn < 0) {
		wn = INT_MAX;
	}

/*
This is wrong. It doesn't have to try and loop while wn is zero
picking up trailing combining characters. This can be simplified. And
it will barf on a trialing illegal
*/
#if HAVE_MBSTATE
	memset(&ps, 0, sizeof(ps));
	while (sn > 0 && wc && (n = mbrtowc(&wc, src, sn, &ps)) != (size_t)-2) {
#else
	while (sn > 0 && wc) {
		n = mbtowc(&wc, src, sn);
#endif
		if (n == (size_t)-1) {
			PMNO(errno);
			return -1;
		}
		if ((w = wcwidth(wc)) < 0) {
			return -1;
		}
		if (w > wn) {
			break;
		}
		wn -= w;
		sn -= n;
		src += n;
		width += w;
	}

	return width;
}

