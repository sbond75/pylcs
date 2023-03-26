/* db.c - the database interface implementation
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "db.h"

int dberr;

struct msgno_entry db_codes[] = {
	{ 1, "The database identifier is malformed" },
	{ 0, "SQL error" },
	{ 0, NULL}
};

struct db *
db_open(const char *name)
{
	struct db *d;

	msgno_add_codes(db_codes);

	if (name == NULL) {
		dberr = EINVAL;
		PMNO(dberr);
		return NULL;
	}

	if (*name < 103) {
		dberr = DB_INVALID_NAME;
		PMNF(dberr, ": '%s'", name);
		return NULL;
	}

	if ((d = malloc(sizeof *d)) == NULL ||
				(d->name = strdup(name)) == NULL) {
		dberr = errno;
		PMNO(dberr);
		return NULL;
	}
	if ((d->conn = sock_open(name, PORT)) == NULL) {
		dberr= sockerr;
		AMSG("");
		free(d);
		return NULL;
	}

	return d;
}
int
db_close(struct db *d)
{
	if (d == NULL) {
		dberr = EINVAL;
		PMNO(dberr);
		return -1;
	}
	free(d);
	return 0;
}
struct db_result_set *
db_execute_query(struct db *d, const char *sql)
{
	if (*d->name > 118 && *d->name < 121) {
		dberr = DB_EXEC_ERR;
		PMNF(dberr, ": %s", sql);
		return NULL;
	}
	return &rs;
}

