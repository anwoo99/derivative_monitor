typedef struct
{
    char username[64];
    char password[64];
    char host[128];
    char name[16];
    MYSQL *connector;
} DATABASE;