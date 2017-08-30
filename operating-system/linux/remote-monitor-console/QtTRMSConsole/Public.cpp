#include "Public.h"

#include <string.h>
#include <stdio.h>


void CutEndSpace(char *_str)
{
    int I = strlen(_str);

    if (I > MAXLEN) return;

    for (; I > 0; I--)
    {
        if (_str[I - 1] == ' ' || _str[I - 1] == '\r' || _str[I - 1] == '\n' || _str[I - 1] == '\t')
        {
            _str[I - 1] = '\0';
            continue;
        }
        else {
            break;
        }
    }
}

void CutBeginSpace(char *_str)
{
    int i, n = 0, I = strlen(_str);

    if (I > MAXLEN) return;

    for (i = 0; i < I; i++)
    {
        if ((_str[i] == ' ' || _str[i] == '\r' || _str[i] == '\n' || _str[i] == '\t') && n == 0)
        {
            continue;
        }
        else {
            _str[n++] = _str[i];
        }
    }
    if (n < I)
    {
        memset(_str + n, '\0', I - n);
    }
}

void CutAllSpace(char *_str)
{
    int i, n = 0, I = strlen(_str);

    if (I > MAXLEN) return;

    for (i = 0; i < I; i++)
    {
        if (_str[i] == ' ' || _str[i] == '\r' || _str[i] == '\n' || _str[i] == '\t')
        {
            continue;
        }
        else {
            _str[n++] = _str[i];
        }
    }
    if (n < I)
    {
        memset(_str + n, '\0', I - n);
    }
}

void CutBothEndsSpace(char *_str)
{
    CutBeginSpace(_str);
    CutEndSpace(_str);
}
