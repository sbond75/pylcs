#include "common.h"
#include "mba/cfg.h"
#include "mba/text.h"
#include "mba/suba.h"

int
user_lookup_by_id(const char *filename, int id, tchar *user)
{
	char buf[0x3FFF]; /* 16K */
	struct allocator *al = suba_init(buf, 0x3FFF, 1, 0);
	struct cfg *cfg = cfg_new(al);

	if (cfg == NULL || cfg_load(cfg, filename) == -1 ||
			cfg_vget_str(cfg, user, 64, NULL, _T("users.%d"), id) == -1) {
		AMSG("");
		return -1;
	}

	/* no cleanup necessary; all memory on stack. */

	return 0;
}

int
CfgSuba(int verbose, struct cfg *cfg, char *args[])
{
	tchar user[64];

	if (user_lookup_by_id(args[0], 104, user) == -1 ||
			tcscoll(user, _T("user0104")) != 0) {
		AMSG("");
		return -1;
	}

	tcase_printf(verbose, "done");

	cfg = NULL;
    return 0;
}
