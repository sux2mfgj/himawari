#include <string.h>

void strcpy(char* dest, const char* src)
{
    int i;
    for(i=0; src[i] != '\0'; ++i){
        dest[i] = src[i];
    }
    dest[i] = '\0';

}

