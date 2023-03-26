#include "common.h"
#include "mba/cfg.h"
#include "mba/domnode.h"

int
DomnodeOps(int verbose, struct cfg *cfg, char *args[])
{
	struct domnode *root, *elem, *attr;
	cfg = NULL;

	root = domnode_new(NULL, NULL, NULL);
	if (domnode_load(root, args[0]) == -1) {
		AMSG("Failed to load XML file: %s", args[0]);
		return -1;
	}

	/* domnode_search
	 */
	if ((elem = domnode_search(root, _T("deny"))) == NULL) {
		AMSG("");
		return -1;
	}
	if (tcscoll(elem->value, _T("interns")) != 0) {
		return -1;
	}

	/* domnode_attrs_put
	 */
	attr = domnode_new(_T("key"), _T("val"), NULL);
	elem = domnode_search(root, _T("export"));
	if (domnode_attrs_put(elem->attrs, attr) == -1) {
		MSG("");
		return -1;
	}

	/* domnode_attrs_get
	 */
	if ((attr = domnode_attrs_get(elem->attrs, _T("accept"))) == NULL) {
		MSG("");
		return -1;
	}
	if (tcscoll(attr->value, _T("auditors")) != 0) {
		MSG("");
		return -1;
	}

	/* domnode_attrs_remove
	 */
	if ((attr = domnode_attrs_remove(elem->attrs, _T("accept"))) == NULL) {
		MSG("");
		return -1;
	}
	if ((domnode_attrs_get(elem->attrs, _T("accept"))) != NULL) {
		MSG("");
		return -1;
	}
	domnode_del(attr);

	if (domnode_store(root, args[1]) == -1) {
		AMSG("Failed to store XML file: %s", args[1]);
		return -1;
	}
	domnode_del(root);

	tcase_printf(verbose, "done");

    return 0;
}
