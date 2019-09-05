#define INC_HELPER

int str_repl(char *buffer, size_t bufsize,const char *string,const char *tbstring,const char *replstring);
int parse_pg_param(size_t *cursize, conf *config,char *conn);
void free_config(conf *config);
int is_alphanum(char *s);
