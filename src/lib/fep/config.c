#include "main.h"

int get_config_json_string(FILE *file, char **json_string)
{
    long file_size;

    /* 파일 크기 계산(바이트 단위) */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *json_string = (char *)malloc(file_size + 1);

    if (!*json_string)
    {
        fclose(file);
        return (-1);
    }

    fread(*json_string, 1, file_size, file);
    (*json_string)[file_size] = '\0';
    fclose(file);

    return 0;
}

int convert_logLevel(char *logLevel)
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
        return (-1);
}

void get_config_json_parsing(CONFIG *config, char *json_string)
{
    char logLevel[32];
    JSON_Value *root_value;
    JSON_Object *root_object;
    JSON_Object *settings_object;
    JSON_Array *ports_array;
    size_t ports_count;

    root_value = json_parse_string(json_string);

    if (!root_value)
    {
        free(json_string);
        return;
    }

    root_object = json_value_get_object(root_value);

    // Parse "settings"
    settings_object = json_object_get_object(root_object, "settings");
    snprintf(config->settings.name, sizeof(config->settings.name), "%s", json_object_get_string(settings_object, "name"));
    snprintf(config->settings.type, sizeof(config->settings.type), "%s", json_object_get_string(settings_object, "type"));
    snprintf(config->settings.desc, sizeof(config->settings.desc), "%s", json_object_get_string(settings_object, "description"));
    snprintf(config->settings.timezone, sizeof(config->settings.timezone), "%s", json_object_get_string(settings_object, "timezone"));
    snprintf(logLevel, sizeof(logLevel), "%s", json_object_get_string(settings_object, "logLevel"));

    if (-1 == (config->settings.logLevel = convert_logLevel(logLevel)))
        return;

    // Parse "ports"
    ports_array = json_object_get_array(root_object, "ports");
    ports_count = json_array_get_count(ports_array);

    for (size_t i = 0; i < ports_count; i++)
    {
        JSON_Object *port_object = json_array_get_object(ports_array, i);
        PORT *port = &config->ports[i];

        snprintf(port->name, sizeof(port->name), "%s", json_object_get_string(port_object, "name"));
        snprintf(port->type, sizeof(port->type), "%s", json_object_get_string(port_object, "type"));
        snprintf(port->desc, sizeof(port->desc), "%s", json_object_get_string(port_object, "description"));

        JSON_Array *watch_array = json_object_get_array(port_object, "watch");
        size_t watch_count = json_array_get_count(watch_array);
        for (size_t j = 0; j < watch_count; j++)
        {
            JSON_Object *watch_object = json_array_get_object(watch_array, j);
            WATCH *watch = &port->watch[j];

            JSON_Object *start_object = json_object_get_object(watch_object, "start");
            JSON_Object *to_object = json_object_get_object(watch_object, "to");

            snprintf(watch->start.wday, sizeof(watch->start.wday), "%s", json_object_get_string(start_object, "wday"));
            snprintf(watch->start.time, sizeof(watch->start.time), "%s", json_object_get_string(start_object, "time"));
            snprintf(watch->to.wday, sizeof(watch->to.wday), "%s", json_object_get_string(to_object, "wday"));
            snprintf(watch->to.time, sizeof(watch->to.time), "%s", json_object_get_string(to_object, "time"));
        }

        JSON_Array *addresses_array = json_object_get_array(port_object, "addresses");
        size_t addresses_count = json_array_get_count(addresses_array);
        for (size_t j = 0; j < addresses_count; j++)
        {
            JSON_Object *address_object = json_array_get_object(addresses_array, j);
            ADDRESS *address = &port->address[j];

            snprintf(address->host, sizeof(address->host), "%s", json_object_get_string(address_object, "host"));
            snprintf(address->nic, sizeof(address->nic), "%s", json_object_get_string(address_object, "nic"));
            snprintf(address->ipad, sizeof(address->ipad), "%s", json_object_get_string(address_object, "ipad"));
            snprintf(address->post, sizeof(address->post), "%s", json_object_get_string(address_object, "port"));
        }
    }
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
        return (-1);

    /* Get the content in xxxx.json */
    if (-1 == get_config_json_string(file, &json_string))
    {
        fclose(file);
        return (-1);
    }

    get_config_json_parsing(config, json_string);

    free(json_string);
    return (0);
}