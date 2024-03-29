#include <stdio.h>
#include <ulfius.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include </usr/include/postgresql/libpq-fe.h>

#include "lohn24pg.h"
#include "helper.h"
#include "parse_config.h"
#include "pg_auth.h"


int close_app=0;
char *conf_path=NULL;

char *user_repl=USER_REPL;
char *hash_repl=HASH_REPL;

PGconn *db_connection=NULL;

void closer(int c){
	close_app=1;
}

void free_config(){
	for(size_t i=0;config.pg_params[i] != NULL;++i){
		free(config.pg_params[i]);
		free(config.pg_values[i]);
	}
	free(config.pg_values);
	free(config.pg_params);
}

int auth_callback (const struct _u_request * request, struct _u_response * response, void * user_data) {
	if(request->auth_basic_user == NULL || request->auth_basic_password == NULL){
		ulfius_set_string_body_response(response, 401, "unauthorized\n");
		return U_CALLBACK_CONTINUE;
	}
	if(strlen(request->auth_basic_user) == 0){
		ulfius_set_string_body_response(response, 401, "unauthorized\n");
		return U_CALLBACK_CONTINUE;
	}
	
	#ifdef DEBUG
		printf("attempt with '%s' | '%s'\n",request->auth_basic_user, request->auth_basic_password);
	#endif
	
	char esc_user[512];
	char esc_user2[512];
	
	str_repl(esc_user2,512,request->auth_basic_user,"\\","\\\\");
	str_repl(esc_user,512,esc_user2,"'","''");

	#ifdef DEBUG	
		printf("escaped user with '%s'\n",esc_user);
	#endif
	
	if(check_password((const char *) esc_user,(const char *) request->auth_basic_password))
		ulfius_set_string_body_response(response, 200, "authorized\n");
	else
		ulfius_set_string_body_response(response, 401, "unauthorized\n");
	return U_CALLBACK_CONTINUE;
}

int main(int ac, char *as[]) {
	signal(SIGINT,closer);
	signal(SIGTERM,closer);
	
	if(ac != 2){
		fprintf(stderr,"Usage: %s [config]\n",as[0]);
		exit(EXIT_FAILURE);
	}

	conf_path=as[1];

	parse_config();
	check_config();
	
	if(!connect_db())
		blame("Error: could not connect to database!\n");
	
	struct _u_instance instance;

	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(config.port);
	inet_aton(config.addr,&addr.sin_addr);
	
	// Initialize instance with the port number
	if (ulfius_init_instance(&instance, config.port, &addr, NULL) != U_OK)
		blame("Error ulfius_init_instance, abort\n");

	ulfius_add_endpoint_by_val(&instance, "GET", config.callback_path, NULL, 0, &auth_callback, NULL);

	if (ulfius_start_framework(&instance) == U_OK) {
		printf("%s started on %d\n",as[0], instance.port);
			
		while(!close_app)
			usleep(500);
	} else
		fprintf(stderr, "Error starting %s\n",as[1]);
	
	puts("stopping...");
	ulfius_stop_framework(&instance);
	ulfius_clean_instance(&instance);
	if(db_connection != NULL)
		PQfinish(db_connection);
	free_config();	
	return 0;
}
