#include "common.h"
#include "mba/pool.h"

int
PoolExercise(int verbose, struct cfg *cfg, char *args[])
{
	void *data;
    int rate, i;
	struct linkedlist *l;
	struct pool *p;
	cfg = NULL; args[0] = NULL;

	if ((p = pool_new(EXERCISE_SM_COUNT,
				(new_fn)allocator_alloc,
				allocator_free,
				NULL,
				NULL, BUFFER_SIZE_SM, 0, NULL)) == NULL ||
			(l = linkedlist_new(0, NULL)) == NULL) {
		PMNO(errno);
		return 1;
	}

	rate = EXERCISE_R0;
	for (i = 0; i < EXERCISE_SM_COUNT; i++) {
		if (i == EXERCISE_SM_P1) {
			rate = EXERCISE_R1;
		} else if (i == EXERCISE_SM_P2) {
			rate = EXERCISE_R2;
		} else if (i == EXERCISE_SM_P3) {
			rate = EXERCISE_R3;
		}

		if (rand() % 10 < rate) {
			if (pool_size(p) == EXERCISE_SM_COUNT && pool_unused(p) == 0) {
				continue;
			} else if ((data = pool_get(p)) == NULL) {
				AMSG("");
				return -1;
			} else if (linkedlist_add(l, data) == -1) {
				AMSG("%04d %p s=%d,u=%d\n", i, data, pool_size(p), pool_unused(p));
				return -1;
			}
			tcase_printf(verbose, "%04d get %p s=%d,u=%d\n", i, data, pool_size(p), pool_unused(p));
		} else if ((data = linkedlist_remove(l, 0))) {
			if (data == NULL || pool_release(p, data) == -1) {
				AMSG("%04d %p s=%d,u=%d\n", i, data, pool_size(p), pool_unused(p));
				return -1;
			}
			tcase_printf(verbose, "%04d rel %p s=%d,u=%d\n", i, data, pool_size(p), pool_unused(p));
		} else {
			tcase_printf(verbose, "%04d nothing to release\n", i);
		}
    }

	linkedlist_del(l, NULL, NULL);
	pool_del(p);

    return 0;
}
