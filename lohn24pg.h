#define INC_LOHN24PG

#include </usr/include/postgresql/libpq-fe.h>

#define USER_REPL "{{username}}"
#define HASH_REPL "{{hash}}"
#define BUFSIZE 2048


//holds constants

char *user_repl;
char *hash_repl;

char *conf_path;

#define CONF_BUF_SIZE 2048
#define PG_MAX 12
#define PG_STS 256


enum { GOT_SQL=1,GOT_PGOPTS=2,GOT_PORT=4,GOT_CALLPATH=8, GOT_ADDR=16};
int conf_flags;

struct {
	char sql_cmd[2048];
	int port;
	char **pg_params;
	char **pg_values;
	char addr[256];
	char callback_path[256];
} config;

//holds db connection
PGconn *db_connection;
int close_app;

void free_config();
