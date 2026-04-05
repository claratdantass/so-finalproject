#include "kutil.h"

void *memset(void *s, int c, unsigned int n)
{
    unsigned char *p = (unsigned char *)s;
    unsigned int i;

    for (i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, unsigned int n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s_ptr = (const unsigned char *)src;
    unsigned int i;

    for (i = 0; i < n; i++) {
        d[i] = s_ptr[i];
    }
    return dest;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

unsigned int strlen(const char *s)
{
    unsigned int len = 0;

    while (s[len]) {
        len++;
    }
    return len;
}
