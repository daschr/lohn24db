#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#define IS_NUM(X) ( 48 <= X && X <= 57 ) 

#ifndef INC_LOHN24PG
	#include "lohn24pg.h"
#endif

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


int is_pos_num(char *s){
	if(*s == '\0')
		return -1;
	char *as=s;
	while(IS_NUM(*as)) ++as;
	if(*as != '\0')
		return -1;
	return atoi(s);
}	

void blame(char *f, ...){
	va_list args;
	va_start(args,f);
	
	vfprintf(stderr,f,args);

	va_end(args); 
	if(db_connection != NULL)
 	      PQfinish(db_connection);
	exit(EXIT_FAILURE);
}

int readline(char *buffer, FILE *fd){
	size_t cpos=0;
	int c;
	for(c=fgetc(fd);cpos < CONF_BUF_SIZE && c != '\n' && c != -1; c=fgetc(fd))
		buffer[cpos++]=c;
	if(cpos==0 && c == -1)
		return 0;
	if(cpos==CONF_BUF_SIZE)
		return 0;
	buffer[cpos]='\0';
	return 1;
}

const char *get_option(int i){
	switch(i){
		case GOT_SQL:	  return "sql_command";
		case GOT_PGOPTS:  return "postgres_options";
		case GOT_PORT:    return "port";
		case GOT_CALLPATH: return "callback_path";
		case GOT_ADDR:	  return "addr";
		default:	  return "unkown";
	}
}
