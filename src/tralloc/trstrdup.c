#include <stdlib.h>
#include <string.h>

char * trstrdup (const char * s)
{
    if (s == NULL)
        return NULL;
    char * ret = malloc(strlen(s) + 1);
    if (ret != NULL)
        strcpy(ret, s);
    return ret;
}
