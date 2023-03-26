/* hexdump - print hexdump of memory to stream
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
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <string.h>
#include "mba/hexdump.h"
#include "defines.h"

#define HEXBUFSIZ 128

#if !defined(__USE_XOPEN)
int mk_wcwidth(wchar_t ucs);
#define wcwidth mk_wcwidth
#endif

#ifdef _WIN32
#define snprintf _snprintf
#endif

void
hexdump(FILE *stream, const void *src, size_t len, size_t width)
{
	unsigned int rows, pos, c, i;
	const char *start, *rowpos, *data;

	data = src;
	start = data;
	pos = 0;
	rows = (len % width) == 0 ? len / width : len / width + 1;
	for (i = 0; i < rows; i++) {
		rowpos = data;
		fprintf(stream, "%05x: ", pos);
		do {
			c = *data++ & 0xff;
			if ((size_t)(data - start) <= len) {
				fprintf(stream, " %02x", c);
			} else {
				fprintf(stream, "   ");
			}
		} while(((data - rowpos) % width) != 0);
		fprintf(stream, "  |");
		data -= width;
		do {
			c = *data++;
			if (isprint(c) == 0 || c == '\t') {
				c = '.';
			}
			if ((size_t)(data - start) <= len) {
				fprintf(stream, "%c", c);
			} else {
				fprintf(stream, " ");
			}
		} while(((data - rowpos) % width) != 0);
		fprintf(stream, "|\n");
		pos += width;
	}
	fflush(stream);
}
int
shexdump(const void *src, size_t len, size_t width, char *dst, char *dlim)
{
	unsigned int rows, pos, c, i;
	const char *start, *dst_start, *rowpos, *data;

	if (dlim <= dst) {
		return 0;
	}
	dlim--;

	start = data = src;
	dst_start = dst;
	pos = 0;
	rows = (len % width) == 0 ? len / width : len / width + 1;
	for (i = 0; i < rows && dst < dlim; i++) {
		rowpos = data;
		dst += snprintf(dst, dlim - dst, "%05x: ", pos);
		do {
			c = *data++ & 0xff;
			if ((size_t)(data - start) <= len) {
				dst += snprintf(dst, dlim - dst, " %02x", c);
			} else {
				dst += snprintf(dst, dlim - dst, "   ");
			}
		} while(((data - rowpos) % width) != 0);
		dst += snprintf(dst, dlim - dst, "  |");
		data -= width;
		do {
			c = *data++;
			if (isprint(c) == 0 || c == '\t') {
				c = '.';
			}
			if ((size_t)(data - start) <= len) {
				dst += snprintf(dst, dlim - dst, "%c", c);
			} else {
				*dst += ' ';
			}
		} while(((data - rowpos) % width) != 0);
		dst += snprintf(dst, dlim - dst, "|\n");
		pos += width;
	}
	*dst = '\0';

	return dst - dst_start;
}

const char *
mbstoax(const char *src, size_t sn, int wn)
{
	static char hexbuf[HEXBUFSIZ];
	char *dst;
	wchar_t ucs;
	int w;
	size_t n;
#if HAVE_MBSTATE > 0
	mbstate_t ps;
#endif

	if (src == NULL) {
		return NULL;
	}

	if (sn > HEXBUFSIZ) {
		sn = HEXBUFSIZ - 1;
	}
	if (wn < 0) {
		wn = HEXBUFSIZ - 1;
	}

	w = 0;
	ucs = 1;
	dst = hexbuf;

#if HAVE_MBSTATE > 0
	memset(&ps, 0, sizeof(ps));
	while (ucs && sn > 0 && (n = mbrtowc(&ucs, src, sn, &ps)) != (size_t)-2) {
		if (n == 0 || n == (size_t)-1 || (w = wcwidth(ucs)) == -1) {
			mbrtowc(0, NULL, 0, &ps);
#else
	while (ucs && sn > 0 && (n = mbtowc(&ucs, src, sn)) != (size_t)-2) {
		if (n == 0 || n == (size_t)-1 || (w = wcwidth(ucs)) == -1) {
			mbtowc(0, NULL, 0);
#endif
			dst += sprintf(dst, "%02x", *src++ & 0xFF);
			sn--;
			continue;
		}
		if (w > wn) {
			break;
		}
		wn -= w;
		sn -= n;
		if (n == 1) {
			*dst = *src;
			src += n;
			dst += n;
		} else {
			while (n--) {
				dst += sprintf(dst, "%02x", *src++ & 0xFF);
			}
		}
	}
	*dst = '\0';

	return hexbuf;
}
