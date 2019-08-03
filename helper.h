#define INC_HELPER

int str_repl(char *buffer, size_t bufsize,const char *string,const char *tbstring,const char *replstring);
int is_pos_num(char *s);
void blame(char *f, ...);
int readline(char *buffer, FILE *fd);
int is_alphanum(char *s);
const char *get_option(int i);
