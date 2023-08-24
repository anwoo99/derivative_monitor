#include "main.h"

int create_directory(char *dirname)
{
    char *token;
    char path[256];
    int result;

    /* 디렉토리 존재여부 검사 */
    if (access(dirname, 0) == 0)
        return (0);

    strcpy(path, dirname);

    token = strtok(path, "/");

    while (token != NULL)
    {
        result = mkdir(path, 0755);

        if (result != 0)
        {
            if (errno != EEXIST)
                return (-1);
        }
        strcat(path, "/");
        strcat(path, token);
        token = strtok(NULL, "/");
    }

    return (0);
}