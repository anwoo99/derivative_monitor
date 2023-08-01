#include "main.h"

int get_database_config(FEP *fep)
{
    char filename[64];
    DATABASE *database = &fep->database;
    long file_size;
    char *json_string;
    FILE *file;

    sprintf(filename, "%s/Database.json", ETC_DIR, fep->exnm);

    file = fopen(filename, "r");

    if (!file)
        return (-1);

    /* Get the content in xxxx.json */
    if (-1 == get_database_json_string(file, &json_string))
    {
        fclose(file);
        return (-1);
    }

    get_database_json_parsing(config, json_string);

    free(json_string);
    return (0);

}

int get_database(FEP *fep)
{
    MYSQL *conn;

    /* Get the configuration from etc/Database.json */
    if (-1 == get_database_config(fep))
    {
        fep_log(fep, FL_ERROR, "Cannot get the database configuration..!");
        return (-1);
    }

    /* Initialize */
    conn = mysql_init(NULL);

    if (!conn)
    {
        fep_log(fep, FL_ERROR, "mysql_init() failed..!");
        return (-1);
    }
}