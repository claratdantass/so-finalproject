/* Funções mínimas em linha para programas em modo usuário (sem libc). */

#ifndef PROGRAMS_ULIB_H
#define PROGRAMS_ULIB_H

/* ---- Comparação e tamanho de string ---- */
static inline int streq(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return *a == *b;
}

static inline unsigned int ustrlen(const char *s)
{
    unsigned int len = 0;

    while (s[len])
        len++;
    return len;
}

/* ---- Inteiro positivo → decimal ASCII (buffer fornecido pelo chamador) ---- */
static inline void itoa(int n, char *buf)
{
    int i = 0, j;
    char tmp;

    if (n == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    buf[i] = '\0';

    for (j = 0; j < i / 2; j++) {
        tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
}

#endif /* PROGRAMS_ULIB_H */
