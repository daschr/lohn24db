#ifndef INC_LOHN24PG
	#include "lohn24pg.h"
#endif

#ifndef INC_HELPER
	#include "helper.h"
#endif

#include </usr/include/postgresql/libpq-fe.h>
int connect_db(void){
        db_connection=PQconnectdbParams((const char * const*)config.pg_params,(const char* const *)config.pg_values,0);
        
        if(db_connection == NULL)
                return 0;
        if(PQstatus(db_connection) != CONNECTION_OK)
                return 1;
        return 0;
}

char *generate_hash(const char *pw){
	return "hui";
}

int check_password(const char * username, const char * password) {
	if(PQstatus(db_connection) != CONNECTION_OK)
		if(!connect_db())
			return 0;
	
	char *hash=generate_hash(password);
	char cmd_buffer1[BUFSIZE];
	char cmd_buffer2[BUFSIZE];
	
	
	if(!str_repl(cmd_buffer1,BUFSIZE,config.sql_cmd,user_repl,username))
		return 0;
	if(!str_repl(cmd_buffer2,BUFSIZE,cmd_buffer2,hash_repl,hash))
		return 0;
	PGresult *res=PQexec(db_connection,cmd_buffer2);
	
	return (PQresultStatus(res) == PGRES_COMMAND_OK);
}
