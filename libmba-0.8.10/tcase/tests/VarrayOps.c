#include "common.h"
#include "mba/suba.h"
#include "mba/varray.h"

struct foo {
	int val;
	char c;
	unsigned long l;
};

/*
267 -> 999
50  -> 55555
15050 -> 7777
*/

int
runnit(struct varray *va, int verbose)
{
	struct foo *foo;
	iter_t iter;

	if ((foo = varray_get(va, 267)) == NULL) {
		MNO(errno);
		return -1;
	}
	foo->val = 999;
	foo = varray_get(va, 50);
	foo->val = 55555;
	foo = varray_get(va, 15050);
	foo->l = 7777;
	varray_release(va, 300);

	varray_iterate(va, &iter);
	while ((foo = varray_next(va, &iter)) != NULL) {
		tcase_printf(verbose, " %d", foo->val);
		if (foo->val == 55555) {
			break;
		}
	}
	if (foo == NULL || foo->val != 55555) {
		PMNO(errno);
		return -1;
	}
	while ((foo = varray_next(va, &iter)) != NULL) {
		tcase_printf(verbose, " %d", foo->val);
		if (foo->val == 999) {
			break;
		}
	}
	if (foo == NULL || foo->val != 999) {
		PMNO(errno);
		return -1;
	}
	while ((foo = varray_next(va, &iter)) != NULL) {
		tcase_printf(verbose, " %d", foo->val);
		if (foo->val) {
			break;
		}
	}
	if (foo != NULL) {
		MSG("foo->val=%d", foo->val);
		return -1;
	}

	return 0;
}
int
VarrayOps(int verbose, struct cfg *cfg, char *args[])
{
	unsigned char mem[0x2FFFF];
	struct allocator *al = suba_init(mem, 0x2FFFF, 1, 0);
	struct varray va1;
	struct varray *va2;

	cfg = NULL; args[0] = NULL;

	if (varray_init(&va1, sizeof(struct foo), al) == -1 ||
			runnit(&va1, verbose) == -1 ||
			varray_deinit(&va1) == -1) {
		MNO(errno);
		return -1;
	}

	if ((va2 = varray_new(sizeof(struct foo), NULL)) == NULL ||
			runnit(va2, verbose) == -1 ||
			varray_del(va2) == -1) {
		MNO(errno);
		return -1;
	}

	tcase_printf(verbose, "\ndone\n");

    return 0;
}
