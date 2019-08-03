#ifndef INC_LOHN24PG
	#include "lohn24pg.h"
#endif

#ifndef INC_HELPER
	#include "helper.h"
#endif

#include </usr/include/postgresql/libpq-fe.h>
#include <openssl/md5.h>
#include <string.h>
void md5_hash(const char *s, char *o){
	unsigned char hash[16];
	MD5((const unsigned char *)s,strlen(s),hash);
	for(int i=0;i<16;++i)
		sprintf(&o[i*2],"%02x",(unsigned int) hash[i]);
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
	
	char hash[33];
	md5_hash(password,hash);
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
	
	return ( res != NULL &&  PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) != 0 );
}
