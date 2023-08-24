#ifndef _CONFIG_H
#define _CONFIG_H


#define MAX_PORT 64
#define MAX_TIME 10

typedef struct
{
    char name[16];
    char type[16];
    char desc[128];
    char timezone[16];
    int room;
    int logLevel;
} SETTINGS;

typedef struct {
    int max_date;
    int depth_log;    
} RAW_DATA;

typedef struct
{
    char start[16];
    char end[16];
} WDAY;

typedef struct
{
    char start[16];
    char end[16];
} WINDOW;

typedef struct
{
    WDAY wday;
    WINDOW window;
} TIME;

typedef struct
{
    int interval;
    time_t last_time;
    unsigned int packet_size_per_check;
    unsigned int total_packet_size;
    int total_check_count;
    double current_speed;
    double total_speed;
    double peak_speed;
    time_t peak_time;
    int prev_checkhour;
    int next_checkhour;
    int curr_checkhour;
} TRAFFIC_TABLE;

typedef struct
{
    int seqn;
    int running;
    char name[16];
    char host[16];
    char nic_name[32];
    char nic_address[32];
    char type[16];
    char format[16];
    char ipad[32];
    int port;
    int intv;
    TIME times[32];
    int sock;
    struct ip_mreq mreq;
    char ipc_name[64];
    unsigned int recv[24];
    unsigned int lost[24];
    TRAFFIC_TABLE traff[24];
} PORT;

typedef struct
{
    SETTINGS settings;
    RAW_DATA raw_data;
    PORT ports[MAX_PORT];
    int nport;
} CONFIG;

#endif
