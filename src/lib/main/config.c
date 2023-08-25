#include "main.h"

#define DEFAULT_INTV 10
#define START_MIN 0
#define END_MIN 59

// Function to convert logLevel string to integer value
int convert_logLevel(const char *logLevel)
{
    if (strcasecmp(logLevel, "MUST") == 0)
        return FL_MUST;
    else if (strcasecmp(logLevel, "ERROR") == 0)
        return FL_ERROR;
    else if (strcasecmp(logLevel, "WARNING") == 0)
        return FL_WARNING;
    else if (strcasecmp(logLevel, "PROGRESS") == 0)
        return FL_PROGRESS;
    else if (strcasecmp(logLevel, "DEBUG") == 0)
        return FL_DEBUG;
    else
        return -1;
}

void get_nic_address(char *nic_address, char *nic_name)
{
    struct ifaddrs *addrs, *tmp;
    struct sockaddr_in *sa;
    char *addr;

    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            sa = (struct sockaddr_in *)tmp->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);

            if (strcmp(tmp->ifa_name, nic_name) == 0)
            {
                strcpy(nic_address, addr);
                break;
            }
        }
        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
    return;
}

// Function to parse JSON string and populate CONFIG structure
void parse_config_json(CONFIG *config, JSON_Value *root_value)
{
    char logLevel[32];
    char running[32];
    JSON_Object *root_object;
    JSON_Object *settings_object;
    JSON_Object *rawdata_object;
    JSON_Array *ports_array;
    size_t ports_count;
    size_t i, j;

    root_object = json_value_get_object(root_value);

    // Parse "settings"
    settings_object = json_object_get_object(root_object, "settings");
    snprintf(config->settings.name, sizeof(config->settings.name), "%s", json_object_get_string(settings_object, "name"));
    snprintf(config->settings.type, sizeof(config->settings.type), "%s", json_object_get_string(settings_object, "type"));
    snprintf(config->settings.desc, sizeof(config->settings.desc), "%s", json_object_get_string(settings_object, "description"));
    snprintf(config->settings.timezone, sizeof(config->settings.timezone), "%s", json_object_get_string(settings_object, "timezone"));
    config->settings.room = (int)json_object_get_number(settings_object, "room");
    snprintf(logLevel, sizeof(logLevel), "%s", json_object_get_string(settings_object, "logLevel"));
    config->settings.logLevel = convert_logLevel(logLevel);

    // Parse "raw_data"
    rawdata_object = json_object_get_object(root_object, "raw_data");
    config->raw_data.max_date = (int)json_object_get_number(rawdata_object, "max_date");
    config->raw_data.depth_log = (int)json_object_get_number(rawdata_object, "depth_log");

    // Parse "ports"
    ports_array = json_object_get_array(root_object, "ports");
    ports_count = json_array_get_count(ports_array);

    config->nport = (ports_count > MAX_PORT) ? MAX_PORT : ports_count;

    for (i = 0; i < ports_count; i++)
    {
        JSON_Object *port_object = json_array_get_object(ports_array, i);
        PORT *port = &config->ports[i];

        port->seqn = i + 1;
        port->running = strcmp(json_object_get_string(port_object, "running"), "ON") == 0 ? true : false;
        port->alert = strcmp(json_object_get_string(port_object, "alert"), "ON") == 0 ? true : false;
        snprintf(port->name, sizeof(port->name), "%s", json_object_get_string(port_object, "name"));
        snprintf(port->host, sizeof(port->host), "%s", json_object_get_string(port_object, "host"));
        snprintf(port->type, sizeof(port->type), "%s", json_object_get_string(port_object, "type"));
        snprintf(port->format, sizeof(port->format), "%s", json_object_get_string(port_object, "format"));
        snprintf(port->ipad, sizeof(port->ipad), "%s", json_object_get_string(port_object, "ipad"));
        port->port = (int)json_object_get_number(port_object, "port");
        snprintf(port->nic_name, sizeof(port->nic_name), "%s", json_object_get_string(port_object, "nic"));

        get_nic_address(port->nic_address, port->nic_name);

        port->intv = (int)json_object_get_number(port_object, "intv");

        if (port->intv == 0)
            port->intv = DEFAULT_INTV;

        sprintf(port->ipc_name, "%s/%s_%s_%s_%d", TMP_DIR, config->settings.name, port->name, port->host, port->seqn);

        JSON_Array *times_array = json_object_get_array(port_object, "times");
        size_t times_count = json_array_get_count(times_array);
        size_t max_times = (times_count > MAX_TIME) ? MAX_TIME : times_count;

        port->ntime = max_times;

        for (j = 0; j < max_times; j++)
        {
            JSON_Object *time_object = json_array_get_object(times_array, j);
            TIME *time_period = &port->times[j];

            JSON_Object *wday_object = json_object_get_object(time_object, "wday");
            JSON_Object *window_object = json_object_get_object(time_object, "window");

            snprintf(time_period->wday.start, sizeof(time_period->wday.start), "%s", json_object_get_string(wday_object, "start"));
            snprintf(time_period->wday.end, sizeof(time_period->wday.end), "%s", json_object_get_string(wday_object, "end"));
            snprintf(time_period->window.start, sizeof(time_period->window.start), "%s", json_object_get_string(window_object, "start"));
            snprintf(time_period->window.end, sizeof(time_period->window.end), "%s", json_object_get_string(window_object, "end"));
        }
    }

    json_value_free(root_value);
}

int _recv_switch_check(CONFIG *config)
{
    int is_check = 0;
    char wdaystr[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    int ii, jj, wday, hour, min;
    PORT *port;
    TIME *time;
    int start_wday, start_hour, start_min;
    int end_wday, end_hour, end_min;

    for (ii = 0; ii < config->nport; ii++)
    {
        port = &config->ports[ii];
        memset(port->recv_switch, 0x00, sizeof(port->recv_switch));

        for (jj = 0; jj < port->ntime; jj++)
        {
            time = &port->times[jj];
            start_wday = -1;
            start_hour = -1;
            start_min = -1;
            end_wday = -1;
            end_hour = -1;
            end_min = -1;

            /* 요일 세팅 */
            for (wday = 0; wday < 7; wday++)
            {
                if (strcmp(wdaystr[wday], time->wday.start) == 0)
                    start_wday = wday;
                if (strcmp(wdaystr[wday], time->wday.end) == 0)
                    end_wday = wday;
            }

            if (start_wday == -1 || end_wday == -1 || end_wday < start_wday)
                return (-1);

            for (wday = start_wday; wday <= end_wday; wday++)
            {
                start_hour = atoi(time->window.start) / 100;
                end_hour = atoi(time->window.end) / 100;

                if (start_hour > end_hour)
                    end_hour += 24;

                for (hour = start_hour; hour <= end_hour; hour++)
                {
                    start_min = START_MIN;
                    end_min = END_MIN;

                    if (hour == start_hour)
                        start_min = atoi(time->window.start) % 100;
                    if (hour == end_hour)
                        end_min = atoi(time->window.end) % 100;

                    for (min = start_min; min <= end_min && min < 60; min++)
                    {
                        port->recv_switch[(wday + 1) % 7][(hour > 24) ? (hour - 24) : hour][min] = 1;
                    }
                }
            }
        }
    }

    return (0);
}

int fep_config(FEP *fep)
{
    char filename[64];
    CONFIG *config = &fep->config;
    long file_size;
    char *json_string;
    FILE *file;
    JSON_Value *rootValue;

    sprintf(filename, "%s/%s.json", ETC_DIR, fep->exnm);

    rootValue = json_parse_file(filename);

    if (!rootValue)
        return (-1);

    parse_config_json(config, rootValue);

    if (-1 == _recv_switch_check(config))
        return (-1);

    return 0;
}
