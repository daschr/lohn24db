#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <jansson.h>
#include <stdint.h>
#include <string.h>
#define IS_NUM(X) ( 48 <= X && X <= 57 ) 
#define IS_ALPHA(X) ((65 <= X && X <= 90) || (97 <= X && X <= 122))

#ifndef INC_LOHN24DB
	#include "lohn24db.h"
#endif


int parse_pg_param(size_t *cursize, conf *config,char *conn){
	if(*cursize > PG_MAX)
		return 0;
	
	if(*cursize == 0){
		*cursize=2;
		config->pg_params=malloc(sizeof(char *)* (*cursize));
		config->pg_values=malloc(sizeof(char *)* (*cursize));
	}else{
		++(*cursize);
		config->pg_params=realloc(config->pg_params,sizeof(char *)* (*cursize));
		config->pg_values=realloc(config->pg_values,sizeof(char *)* (*cursize));
	}
	
	size_t last_elem=*cursize-1, plast_elem=*cursize-2;
	
	config->pg_params[last_elem]=NULL;
	config->pg_values[last_elem]=NULL;
	
	size_t pos=0;
	for(;conn[pos] != '\0' && conn[pos]!=':';++pos);
	if(conn[pos]!=':')
		return 0;

	config->pg_params[plast_elem]=malloc((pos+2)*sizeof(char));
	strncpy(config->pg_params[plast_elem],conn,pos);
	config->pg_params[plast_elem][pos]='\0';
	
	config->pg_values[plast_elem]=strdup(conn+pos+1);
	#ifdef DEBUG
		printf("pg_param: '%s' pg_value: '%s'\n",config->pg_params[plast_elem],config->pg_values[plast_elem]);
	#endif
	
	return 1;
}


int is_alphanum(char *s){
	for(size_t i=0;s[i] != '\0';++i)
		if(!IS_NUM(s[i]) && !IS_ALPHA(s[i]))
			return 0;
	return 1;
}

void free_config(conf *config){
	for(size_t i=0;config->pg_params[i] != NULL;++i){
		free(config->pg_params[i]);
		free(config->pg_values[i]);
	}
	free(config->pg_values);
	free(config->pg_params);
	free(config);
}

int str_repl(char *buffer, size_t bufsize,const char *string,const char *tbstring,const char *replstring){
//        printf("%s | %s | %s \n",string,tbstring,replstring);
	size_t strpos=0;
        size_t bufferpos=0;
        size_t tbstringl=strlen(tbstring);
        while(string[strpos] != '\0' && bufferpos < bufsize){
                if(string[strpos]==*tbstring){
                        if(strncmp(string+strpos,tbstring,tbstringl)==0){
                                for(size_t i=0;i<strlen(replstring);++i){
                        	          if(bufferpos==bufsize-1)
                                                return 0;
                                        buffer[bufferpos++]=replstring[i];
                                }
                                strpos+=tbstringl;
                        }else
                                buffer[bufferpos++]=string[strpos++];
                }else
                        buffer[bufferpos++]=string[strpos++];
        }
        if(bufferpos>=bufsize)
                return 0;
        buffer[bufferpos]='\0';
        return 1;
}

/*
void blame(char *f, ...){
	va_list args;
	va_start(args,f);
	
	vfprintf(stderr,f,args);

	va_end(args); 
	if(db_connection != NULL)
 	      PQfinish(db_connection);
	free_config();
	exit(EXIT_FAILURE);
}

json_t *js_blame(char *f, ...){
	va_list args;
	va_start(args,f);
	size_t comp_s=strlen(f);
	
	for(size_t i=0;i<sizeof(args)/sizeof(char*);++i)
		comp_s+=strlen(args[i]);
	++comp_s;
	
	char s[comp_s];
	vsprintf(s,f,args);
	json_t js=json_pack("{sis[s]}", "result", G_ERROR_PARAM, "error", s)	
	
	va_end(args);

	return s;
}*/
