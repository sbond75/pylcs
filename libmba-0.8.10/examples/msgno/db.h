/* db.h - a database interface
 */

#ifndef DB_H
#define DB_H

#include <mba/msgno.h>
#include "sock.h"

extern int dberr;
extern struct msgno_entry db_codes[];

#define DB_INVALID_NAME db_codes[0].msgno
#define DB_EXEC_ERR     db_codes[1].msgno

#define PORT 900

struct db {
	const char *name;
	struct sock *conn;
};

struct db_result_set {
} rs;

struct db *db_open(const char *name);
int db_close(struct db *d);
struct db_result_set *db_execute_query(struct db *d, const char *sql);

#endif /* DB_H */
