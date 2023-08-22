#include "main.h"

#define DEFAULT_INTV 10

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

// Function to parse JSON string and populate CONFIG structure
void parse_config_json(CONFIG *config, char *json_string)
{
    char logLevel[32];
    char running[32];
    JSON_Value *root_value;
    JSON_Object *root_object;
    JSON_Object *settings_object;
    JSON_Array *ports_array;
    size_t ports_count;
    size_t i, j;

    root_value = json_parse_string(json_string);

    if (!root_value)
    {
        return;
    }

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
        snprintf(port->name, sizeof(port->name), "%s", json_object_get_string(port_object, "name"));
        snprintf(port->type, sizeof(port->type), "%s", json_object_get_string(port_object, "type"));
        snprintf(port->format, sizeof(port->format), "%s", json_object_get_string(port_object, "format"));
        snprintf(port->ipad, sizeof(port->ipad), "%s", json_object_get_string(port_object, "ipad"));
        port->port = (int)json_object_get_number(port_object, "port");
        port->intv = (int)json_object_get_number(port_object, "intv");

        if (port->intv == 0)
            port->intv = DEFAULT_INTV;

        sprintf(port->ipc_name, "%s/%s_%s_%s_%d", TMP_DIR, config->settings.name, port->name, port->host, port->seqn);

        JSON_Array *times_array = json_object_get_array(port_object, "times");
        size_t times_count = json_array_get_count(times_array);
        size_t max_times = (times_count > MAX_TIME) ? MAX_TIME : times_count;

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

int fep_config(FEP *fep)
{
    char filename[64];
    CONFIG *config = &fep->config;
    long file_size;
    char *json_string;
    FILE *file;

    sprintf(filename, "%s/%s.json", ETC_DIR, fep->exnm);

    file = fopen(filename, "r");

    if (!file)
        return -1;

    /* Get the content in xxxx.json */
    if (-1 == get_config_json_string(file, &json_string))
    {
        fclose(file);
        return -1;
    }

    parse_config_json(config, json_string);

    free(json_string);
    fclose(file);
    return 0;
}
