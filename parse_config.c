#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef INC_LOHN24PG
	#define INC_LOHN24PG
	#include "lohn24pg.h"
#endif

#ifndef INC_HELPER
	#define INC_HELPER
	#include "helper.h"
#endif

#define IS(X) if(strcmp(parm,X)==0)
#define is_empty(X) if(X[0]=='\0')

int parse_pg_param(char *conf);

void parse_config(){
	FILE *conf=fopen(conf_path,"r");
	if(conf == NULL)
		blame("Error: could not open config file: '%s'!\n",conf_path);

	char buffer[CONF_BUF_SIZE], parm[CONF_BUF_SIZE],nparm[CONF_BUF_SIZE];
	size_t pos=0;	
	while(readline(buffer,conf)){
		if(buffer[0]=='\0' || buffer[0]=='#')
			continue;
		#ifdef DEBUG
		printf("parsing: '%s'\n",buffer);
		#endif
		for(pos=0;buffer[pos]!='\0' && buffer[pos]!='=';++pos);
		if(buffer[pos]=='\0'){
			fclose(conf);
			blame("Error: option '%s' in config file is invalid!\n",buffer);
		}
		strncpy(parm,buffer,pos);
		parm[pos]='\0';
		if(buffer[pos+1]=='\0'){
			fclose(conf);
			blame("Error: option '%s' in config file is empty\n",parm);
		}
		strcpy(nparm,buffer+pos+1);
		#ifdef  DEBUG
		printf("parm: '%s' nparm: '%s'\n",parm,nparm);
		#endif
		
		IS("sql_cmd"){
			strncpy(config.sql_cmd,nparm,2048);
			conf_flags^=GOT_SQL;
		}else IS("callback_path"){
			strncpy(config.callback_path,nparm,256);
			conf_flags^=GOT_CALLPATH;
		}else IS("addr"){
			strncpy(config.addr,nparm,256);
			conf_flags^=GOT_ADDR;
		}else IS("port"){
			if((config.port=is_pos_num(nparm)) == -1){
				fclose(conf);
				blame("Error: option '%s' must be a number!\n",parm);
			}
			conf_flags^=GOT_PORT;
		}else IS("postgres_options"){
			for(char *s=strtok(nparm,",");s!=NULL;s=strtok(NULL,","))
				if(!parse_pg_param(s))
					blame("Error: cannot parse '%s'!\n",s);
			conf_flags^=GOT_PGOPTS;
		}else{
			fclose(conf);
			blame("Error: unkown option '%s' in config file!\n",parm);
		}
	}
	fclose(conf); 
}

void check_config(){
	for(int i=1;i<32;i=i<<1)
		if(! ( conf_flags & i ))
			blame("Error: missing option '%s' in config file!\n",get_option(i));	
}

int parse_pg_param(char *conf){
	static size_t cursize=0;	
	static size_t rnum=0;
	if(rnum == PG_MAX)
		return 0;
	
	if(cursize == 0){
		cursize+=2;
		config.pg_params=malloc(sizeof(char *)*cursize);
		config.pg_values=malloc(sizeof(char *)*cursize);
	}else{
		++cursize;
		config.pg_params=realloc(config.pg_params,sizeof(char *)*cursize);
		config.pg_values=realloc(config.pg_values,sizeof(char *)*cursize);
	}
	config.pg_params[rnum+1]=NULL;
	config.pg_values[rnum+1]=NULL;
	
	printf("rnum: %lu\n",rnum);
	
	size_t pos=0;
	for(;conf[pos] != '\0' && conf[pos]!=':';++pos);
	if(conf[pos]!=':')
		return 0;

	config.pg_params[rnum]=malloc((pos+2)*sizeof(char));
	strncpy(config.pg_params[rnum],conf,pos);
	config.pg_params[rnum][pos]='\0';
	
	config.pg_values[rnum]=strdup(conf+pos+1);
	#ifdef DEBUG
		printf("pg_param: '%s' pg_value: '%s'\n",config.pg_params[rnum],config.pg_values[rnum]);
	#endif
	++rnum;
	return 1;
}

