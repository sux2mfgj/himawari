#include "string.h"

bool h_memcmp(const void *s1, const void *s2, size_t n)
{
    char *left = (char *)s1;
    char *right = (char *)s2;
    for (int i = 0; i < n; ++i)
    {
        if(*left != *right)
        {
            return false;
        }
    }

    return true;
}
