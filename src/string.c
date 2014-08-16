#include "string.h"


void memset(void* ptr, char c, size_t size)
{
    char* tmp = (char *)ptr;
    for(int i = 0; i < size; ++i){
        tmp[i] = c;
    }
}
