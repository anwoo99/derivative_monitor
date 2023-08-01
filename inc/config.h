typedef struct
{
    char name[16];
    char type[16];
    char desc[128];
    char timezone[16];
    int logLevel;
} SETTINGS;

typedef struct
{
    char wday[16];
    char time[16];
} WATCH_TIME;

typedef struct
{
    WATCH_TIME start;
    WATCH_TIME to;
} WATCH;

typedef struct
{
    char host[32];
    char nic[32];
    char ipad[32];
    char post[32];
} ADDRESS;

typedef struct
{
    char name[16];
    char type[16];
    char desc[128];
    WATCH watch[8];
    ADDRESS address[32];
} PORT;

typedef struct
{
    SETTINGS settings;
    PORT ports[32];
} CONFIG;