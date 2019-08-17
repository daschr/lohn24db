#define INC_LOHN24DB

#include </usr/include/postgresql/libpq-fe.h>

#define USER_REPL "{{username}}"
#define HASH_REPL "{{hash}}"
#define BUFSIZE 2048


//holds constants

const char *user_repl;
const char *hash_repl;
char *pg_params[5];
char *pg_values[5];
	
#define PG_MAX 12
#define PG_STS 256

typedef struct {
	char sql_cmd[BUFSIZE];
	PGconn *db_connection;
} conf;

