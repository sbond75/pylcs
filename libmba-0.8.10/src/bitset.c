/* bitset - a set of bits
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

#include <errno.h>
#include "mba/msgno.h"
#include "mba/bitset.h"

int
bitset_find_first(void *ptr, void *plim, int val)
{
	unsigned char *start = ptr;
	unsigned char *bs;
	int cond = val ? 0x00 : 0xFF;

	for (bs = start; bs < (unsigned char *)plim; bs++) {
		if ((*bs & 0xFF) != cond) {
			int b = *bs & 0xFF;

				/* Index of the first bit that is 0 if
				 * val is 0 or 1 if val is 1.
				 */
			if (!val) {
				b = ~b;
			}
			b = b & -b;
			switch (b) {
				case 1: b = 0; break;
				case 2: b = 1; break;
				case 4: b = 2; break;
				case 8: b = 3; break;
				case 16: b = 4; break;
				case 32: b = 5; break;
				case 64: b = 6; break;
				case 128: b = 7; break;
			}

			return b + (bs - start) * 8;
		}
	}

	errno = ENOENT;
	PMNO(errno);
	return -1;
}
void
bitset_iterate(iter_t *iter)
{
	iter->i1 = 0;
	iter->i2 = 0x01;
}
int
bitset_next(void *ptr, void *plim, iter_t *iter)
{
	unsigned char *b = (unsigned char *)ptr + iter->i1;
	int ret = -1;

	if (b < (unsigned char *)plim) {
		ret = (*b & iter->i2) != 0;

		if (iter->i2 == 0x80) {
			iter->i1++;
			iter->i2 = 0x01;
		} else {
			iter->i2 <<= 1;
		}
	}

	return ret;
}
