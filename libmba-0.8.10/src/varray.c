/* varray - a variable sized array
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
#include <string.h>
#include <errno.h>
#include "mba/iterator.h"
#include "mba/varray.h"
#include "defines.h"

#define AL(va) (((struct varray *)(va))->al ? (char *)(va) - ((struct varray *)(va))->al : 0)
#define BINADDR(va,i) ((va)->bins[i] ? AL(va) + (va)->bins[i] : 0)

int
varray_init(struct varray *va, size_t membsize, struct allocator *al)
{
	if (va == NULL || membsize == 0) {
		errno = EINVAL;
		return -1;
	}

	memset(va, 0, sizeof *va);
	va->size = membsize;
	if (al && al->tail) { /* al is a suba allocator */
		va->al = (char *)va - (char *)al;
	}

	return 0;
}
int
varray_reinit(struct varray *va, struct allocator *al)
{
	if (al && al->tail) { /* al is a suba allocator */
		va->al = (char *)va - (char *)al;
	}
	return 0;
}
int
varray_deinit(struct varray *va)
{
	varray_release(va, 0);
	return 0;
}
struct varray *
varray_new(size_t membsize, struct allocator *al)
{
	struct varray *va;

	if ((va = allocator_alloc(al, sizeof *va, 0)) == NULL) {
		return NULL;
	}
	if (varray_init(va, membsize, al) == -1) {
		allocator_free(al, va);
		return NULL;
	}

	return va;
}
int
varray_del(void *va)
{
	int ret = 0;

	if (va) {
		struct allocator *al = (struct allocator *)AL(va);
		ret += varray_release(va, 0);
		ret += allocator_free(al, va);
	}

	return ret;
}
int
varray_release(struct varray *va, unsigned int from)
{
	unsigned int r, i;
	int ret = 0;

	if (va == NULL) {
		return 0;
	}

	r = (1 << VARRAY_INIT_SIZE);
	for (i = 0; i < 16; i++) {
		if (from <= r) {
			break;
		}
		r *= 2;
	}
	if (from != 0) i++;
	for ( ; i < 16; i++) {
		if (va->bins[i]) {
			ret += allocator_free(AL(va), BINADDR(va, i));
			va->bins[i] = 0;
		}
	}

	return ret ? -1 : 0;
}
void *
varray_get(struct varray *va, unsigned int idx)
{
	unsigned int r, i, n;

	if (va == NULL) {
		errno = EINVAL;
		return NULL;
	}

	r = (1 << VARRAY_INIT_SIZE);          /* First and second bins hold 32 then 64,128,256,... */
	for (i = 0; i < 16; i++) {
		if (r > idx) {
			break;
		}
		r *= 2;
	}
	if (i == 16) {
		errno = ERANGE;
		return NULL;
	}

	n = i == 0 ? (1 << VARRAY_INIT_SIZE) : 1 << (i + (VARRAY_INIT_SIZE - 1)); /* n is nmemb in bin i */

	if (va->bins[i] == 0) {
		char *mem = allocator_alloc((struct allocator *)AL(va), n * va->size, 1);
		if (mem == NULL) {
			return NULL;
		}
		va->bins[i] = mem - AL(va);
	}

	return (char *)BINADDR(va, i) + (i == 0 ? idx : idx - n) * va->size;
}
void
varray_iterate(void *va, iter_t *iter)
{
	if (va && iter) {
		iter->i1 = iter->i2 = 0;
	}
}
void *
varray_next(void *va, iter_t *iter)
{
	struct varray *va0 = va;
	unsigned int n;

	if (va == NULL || iter == NULL) {
		return NULL;
	}

	n = iter->i1 == 0 ? (1 << VARRAY_INIT_SIZE) : 1 << (iter->i1 + (VARRAY_INIT_SIZE - 1)); /* n is nmemb in iter->i1 */

	if (iter->i2 == n) {
		iter->i2 = 0;
		iter->i1++;
	}

	while (BINADDR(va0, iter->i1) == NULL) {
		iter->i1++;
		if (iter->i1 == 16) {
			return NULL;
		}
	}

	return BINADDR(va0, iter->i1) + iter->i2++ * va0->size;
}

