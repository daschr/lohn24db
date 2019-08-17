#define INC_PG_AUTH
int connect_db(conf *config);
        
int check_password(conf *config, const char * username, const char * password);
