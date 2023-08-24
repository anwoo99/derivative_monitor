#include "main.h"

int create_directory(char *dirname)
{
    char *token;
    char path[256];
    int result;
    char *next_ptr;
    char full_path[256];

    /* 디렉토리 존재여부 검사 */
    if (access(dirname, 0) == 0)
        return (0);

    strcpy(path, dirname);
    strcpy(full_path, ""); // Initialize the full_path string

    token = strtok_r(path, "/", &next_ptr);

    while (token != NULL)
    {
        // Append the current subdirectory to the full path
        strcat(full_path, "/");
        strcat(full_path, token);

        if (strlen(full_path) > strlen(LOG_DIR))
        {
            result = mkdir(full_path, 0755);

            if (result != 0)
            {
                if (errno != EEXIST)
                    return (-1);
            }
        }

        token = strtok_r(NULL, "/", &next_ptr);
    }

    return (0);
}
