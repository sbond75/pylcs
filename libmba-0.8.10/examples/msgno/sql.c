/* mydb.c - an sql program that queries a database
 */

#include <stdlib.h>
#include <string.h>
#include "db.h"

struct msgno_entry mydb_codes[] = {
	{ 1, "Must provide a database name like 'www', 'bar', or 'hal', etc" },
	{ 0, "Failed to open database" },
	{ 0, "Failed to close database" },
	{ 0, "Database transaction failed" },
	{ 0, NULL }
};

#define MYDB_NO_ARG    mydb_codes[0].msgno
#define MYDB_OPEN_ERR  mydb_codes[1].msgno
#define MYDB_CLOSE_ERR mydb_codes[2].msgno
#define MYDB_TRANS_ERR mydb_codes[3].msgno

int
main(int argc, char *argv[])
{
	struct db *d;
	struct db_result_set *rs;

	msgno_add_codes(mydb_codes);

	if (argc < 2) {
		MNO(MYDB_NO_ARG);
		return EXIT_FAILURE;
	}

	d = db_open(argv[1]);
	if (d == NULL) {
		MNF(MYDB_OPEN_ERR, " '%s'", argv[1]);
		return EXIT_FAILURE;
	}

	rs = db_execute_query(d, "select * from emp");
	if (rs == NULL) {
		MNO(MYDB_TRANS_ERR);
		db_close(d);
		return EXIT_FAILURE;
	}

	if (db_close(d) == -1) {
		MNF(MYDB_CLOSE_ERR, " '%s'", argv[1]);
		return EXIT_FAILURE;
	}

	MNO(0); /* Success */

	return EXIT_SUCCESS;
}
