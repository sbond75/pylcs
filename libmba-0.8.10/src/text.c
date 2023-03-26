/* text - uniform multi-byte/wide text handling
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
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <wchar.h>
#include <mba/msgno.h>
#include <mba/text.h>
#include "defines.h"

int
str_length(const unsigned char *src, const unsigned char *slim)
{
	const unsigned char *start = src;

	if (src == NULL || src >= slim) {
		return 0;
	}
	while (*src) {
		src++;
		if (src == slim) {
			return 0;
		}
	}

	return src - start;
}
int
wcs_length(const wchar_t *src, const wchar_t *slim)
{
	const wchar_t *start = src;

	if (src == NULL || src >= slim) {
		return 0;
	}
	while (*src) {
		src++;
		if (src == slim) {
			return 0;
		}
	}

	return src - start;
}
size_t
str_size(const unsigned char *src, const unsigned char *slim)
{
	const unsigned char *start = src;

	if (src == NULL || src >= slim) {
		return 0;
	}
	while (*src) {
		src++;
		if (src == slim) {
			return 0;
		}
	}

	return (src - start + 1) * sizeof *src;
}
size_t
wcs_size(const wchar_t *src, const wchar_t *slim)
{
	const wchar_t *start = src;

	if (src == NULL || src >= slim) {
		return 0;
	}
	while (*src) {
		src++;
		if (src == slim) {
			return 0;
		}
	}

	return (src - start + 1) * sizeof *src;
}
int
str_copy(const unsigned char *src, const unsigned char *slim,
		unsigned char *dst, unsigned char *dlim, int n)
{
	unsigned char *start = dst;

	if (dst == NULL || dst >= dlim) {
		return 0;
	}
	if (src == NULL || src >= slim) {
		*dst = '\0';
		return 0;
	}
	while (n-- && *src) {
		*dst++ = *src++;
		if (src == slim || dst == dlim) {
			dst = start;
			break;
		}
	}
	*dst = '\0';

	return dst - start;
}
int
wcs_copy(const wchar_t *src, const wchar_t *slim,
		wchar_t *dst, wchar_t *dlim, int n)
{
	wchar_t *start = dst;

	if (dst == NULL || dst >= dlim) {
		return 0;
	}
	if (src == NULL || src >= slim) {
		*dst = L'\0';
		return 0;
	}
	while (n-- && *src) {
		*dst++ = *src++;
		if (src == slim || dst == dlim) {
			dst = start;
			break;
		}
	}
	*dst = L'\0';

	return dst - start;
}
int
str_copy_new(const unsigned char *src,
	const unsigned char *slim,
	unsigned char **dst,
	int n,
	struct allocator *al)
{
	const unsigned char *start = src;
	size_t siz;

	if (dst == NULL) {
		return 0;
	}
	if (src == NULL || src >= slim) {
		*dst = NULL;
		return 0;
	}
	while (n-- && *src) {
		src++;
		if (src == slim) {
			*dst = NULL;
			return 0;
		}
	}
	siz = src - start + 1;
	if ((*dst = allocator_alloc(al, siz, 0)) == NULL) {
		return -1;
	}
	memcpy(*dst, start, siz);
	(*dst)[src - start] = '\0';

	return src - start;
}
int
wcs_copy_new(const wchar_t *src,
	const wchar_t *slim,
	wchar_t **dst,
	int n,
	struct allocator *al)
{
	const wchar_t *start = src;
	size_t siz;

	if (dst == NULL) {
		return 0;
	}
	if (src == NULL || src >= slim) {
		*dst = NULL;
		return 0;
	}
	while (n-- && *src) {
		src++;
		if (src == slim) {
			*dst = NULL;
			return 0;
		}
	}
	siz = (src - start + 1) * sizeof *src;
	if ((*dst = allocator_alloc(al, siz, 0)) == NULL) {
		return -1;
	}
	memcpy(*dst, start, siz);
	(*dst)[src - start] = L'\0';

	return src - start;
}
/* Standard UTF-8 decoder
 */
int
utf8towc(const unsigned char *src, const unsigned char *slim, wchar_t *wc)
{
	const unsigned char *start = src;
	ptrdiff_t n = slim - src;

	if (n < 1) return 0;

	if (*src < 0x80) {
		*wc = *src;
	} else if ((*src & 0xE0) == 0xC0) {
		if (n < 2) return 0;
		*wc = (*src++ & 0x1F) << 6;
		if ((*src & 0xC0) != 0x80) {
			errno = EILSEQ;
			return -1;
		} else {
			*wc |= *src & 0x3F;
		}
		if (*wc < 0x80) {
			errno = EILSEQ;
			return -1;
		}
	} else if ((*src & 0xF0) == 0xE0) {
		if (n < 3) return 0;
		if (sizeof *wc < 3) {
			errno = EILSEQ; /* serrogates not supported */
			return -1;
		}
		*wc = (*src++ & 0x0F) << 12;
		if ((*src & 0xC0) != 0x80) {
			errno = EILSEQ;
			return -1;
		} else {
			*wc |= (*src++ & 0x3F) << 6;
			if ((*src & 0xC0) != 0x80) {
				errno = EILSEQ;
				return -1;
			} else {
				*wc |= *src & 0x3F;
			}
		}
		if (*wc < 0x800) {
			errno = EILSEQ;
			return -1;
		}
	} else if ((*src & 0xF8) == 0xF0) {
		if (n < 4) return 0;
		*wc = (*src++ & 0x07) << 18;
		if ((*src & 0xC0) != 0x80) {
			errno = EILSEQ;
			return -1;
		} else {
			*wc |= (*src++ & 0x3F) << 12;
			if ((*src & 0xC0) != 0x80) {
				errno = EILSEQ;
				return -1;
			} else {
				*wc |= (*src++ & 0x3F) << 6;
				if ((*src & 0xC0) != 0x80) {
					errno = EILSEQ;
					return -1;
				} else {
					*wc |= *src & 0x3F;
				}
			}
		}
		if (*wc < 0x10000) {
			errno = EILSEQ;
			return -1;
		}
	}
	src++;

	return src - start;
}

int
_fputws(const wchar_t *buf, FILE *stream)
{
	char mb[16];
	int n = 0;

#if __USE_UNIX98
	mbstate_t ps;
    memset(&ps, 0, sizeof(ps));
    while (*buf) {
		if ((n = wcrtomb(mb, *buf, &ps)) == -1) {
#else
    while (*buf) {
		if ((n = wctomb(mb, *buf)) == -1) {
#endif
			PMNO(errno);
			return -1;
		}
		if (fwrite(mb, n, 1, stream) != 1) {
			PMNO(errno);
			return -1;
		}
		buf++;
	}
	return 0;
}

#if !defined(__USE_GNU)
#if HAVE_STRDUP < 1
char *
strdup(const char *s)
{
	return s ? strcpy(malloc(strlen(s) + 1), s) : NULL;
}
wchar_t *
wcsdup(const wchar_t *s)
{
	return s ? wcscpy(malloc((wcslen(s) + 1) * sizeof *s), s) : NULL;
}
#endif

size_t
strnlen(const char *s, size_t maxlen)
{
	size_t len;
	for (len = 0; *s && len < maxlen; s++, len++);
	return len;
}
/* int */
/* vsnprintf(char *str, size_t size, const char *format, va_list ap) */
/* { */
/* 	size = 0; */
/* 	return vsprintf(str, format, ap); */
/* } */

size_t
wcsnlen(const wchar_t *s, size_t maxlen)
{
	size_t len;
	for (len = 0; *s && len < maxlen; s++, len++);
	return len;
}
int
wcscasecmp(const wchar_t *s1, const wchar_t *s2)
{
	wchar_t c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;
		if (c1 == L'\0' || c2 == L'\0') {
			break;
		}
		if (c1 != c2) {
			c1 = towupper(c1);
			c2 = towupper(c2);
		}
	} while (c1 == c2);

	return c1 - c2;
}

#endif

