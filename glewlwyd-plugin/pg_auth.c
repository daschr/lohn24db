#ifndef INC_LOHN24DB
	#include "lohn24db.h"
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

int connect_db(conf *config){
	if(config->db_connection != NULL)
		PQfinish(config->db_connection);
	
	printf("%s\n",pg_params[0]);
	for(size_t i=0; pg_params[i] != NULL; ++i)
		printf("%s %s\n",pg_params[i],pg_values[i]);
	config->db_connection=PQconnectdbParams((const char * const*)pg_params,(const char* const *)pg_values,0);
        if(config->db_connection == NULL)
                return 0;
        if(PQstatus(config->db_connection) == CONNECTION_OK)
                return 1;
        return 0;
}

int check_password(conf *config, const char * username, const char * password) {
	if(PQstatus(config->db_connection) != CONNECTION_OK)
		if(!connect_db(config))
			return 0;
	
	struct crypt_data d;
	memset(&d,0,sizeof(d));

	char *hash;
	if((hash=md5_hash(username,password,&d)) == NULL)
		return 0;

	char cmd_buffer1[BUFSIZE];
	char cmd_buffer2[BUFSIZE];
	
	
	if(!str_repl(cmd_buffer1,BUFSIZE,config->sql_cmd,user_repl,username))
		return 0;

	if(!str_repl(cmd_buffer2,BUFSIZE,cmd_buffer1,hash_repl,hash))
		return 0;
	
	#ifdef DEBUG
		printf("cmd_buffer2: %s\n",cmd_buffer2);
	#endif
	PGresult *res=PQexec(config->db_connection,cmd_buffer2);
	
	int comp= (res != NULL &&  PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) != 0 );
	if(res != NULL)
		PQclear(res);
	return comp;
}
