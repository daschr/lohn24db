#ifndef INC_LOHN24PG
	#include "lohn24pg.h"
#endif

#ifndef INC_HELPER
	#include "helper.h"
#endif

#include </usr/include/postgresql/libpq-fe.h>
#include <string.h>
#include <stdlib.h>
#define __USE_GNU
#include <crypt.h>

char *md5_hash(const char *user,const char *pw,struct crypt_data *d){
	char salt[4+strlen(user)];
	strcpy(salt,"$1$");
	strcpy(salt+3,user);
	return crypt_r(pw,salt,d);
}

int connect_db(void){
	if(db_connection != NULL)
		PQfinish(db_connection);
	db_connection=PQconnectdbParams((const char * const*)config.pg_params,(const char* const *)config.pg_values,0);
        if(db_connection == NULL)
                return 0;
        if(PQstatus(db_connection) == CONNECTION_OK)
                return 1;
        return 0;
}

int check_password(const char * username, const char * password) {
	if(PQstatus(db_connection) != CONNECTION_OK)
		if(!connect_db())
			return 0;
	
	struct crypt_data d;
	memset(&d,0,sizeof(d));

	char *hash;
	if((hash=md5_hash(username,password,&d)) == NULL)
		return 0;

	char cmd_buffer1[BUFSIZE];
	char cmd_buffer2[BUFSIZE];
	
	
	if(!str_repl(cmd_buffer1,BUFSIZE,config.sql_cmd,user_repl,username))
		return 0;

	if(!str_repl(cmd_buffer2,BUFSIZE,cmd_buffer1,hash_repl,hash))
		return 0;
	
	#ifdef DEBUG
		printf("cmd_buffer2: %s\n",cmd_buffer2);
	#endif
	PGresult *res=PQexec(db_connection,cmd_buffer2);
	
	int comp= (res != NULL &&  PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) != 0 );
	if(res != NULL)
		PQclear(res);
	return comp;
}
