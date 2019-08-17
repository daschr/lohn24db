#define INC_HELPER
#include <jansson.h>

int str_repl(char *buffer, size_t bufsize,const char *string,const char *tbstring,const char *replstring);
int is_pos_num(char *s);
int parse_pg_param(conf *config,char *conn);
void free_config(conf *config);
void blame(char *f, ...);
json_t *js_blame(char *f, ...);
int readline(char *buffer, FILE *fd);
int is_alphanum(char *s);
const char *get_option(int i);
