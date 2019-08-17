#define INC_LOHN24DB

#include </usr/include/postgresql/libpq-fe.h>

#define USER_REPL "{{username}}"
#define HASH_REPL "{{hash}}"
#define BUFSIZE 2048


//holds constants

const char *user_repl;
const char *hash_repl;

#define PG_MAX 12
#define PG_STS 256

typedef struct {
	char sql_cmd[BUFSIZE];
	char **pg_params;
	char **pg_values;
	PGconn *db_connection;
} conf;

