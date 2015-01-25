#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "strings.h"

int is_empty(const char *src)
{
    while (*src && isspace(*src++));
    return *src == '\0';
}

char *ltrim(const char *str)
{
    const char *p = str;
    
    while (*p && isspace(*p)) {
        p++;
    }
    
    return (char *)p;
}

char *rtrim(char *str){
    char *p;
    p = str + strlen(str) - 1;
    while(p >= str && (isspace(*p) || *p=='\n' || *p == '\r')){
        p--;
    }
    *(++p) = '\0';
    return p;
}


char *trim(const char *str)
{
    char *p;
    p = ltrim(str);
    rtrim(p);
    return p;
}

char *substr(const char *str, size_t start, size_t len)
{
    size_t size;
    if (len == -1) {
        size = strlen(str) - start;
    } else {
        size = len;
    }
    
    char *ret = (char *)malloc(sizeof(char) * size + 1);
    assert(ret);
    memset(ret, 0, sizeof(*ret));
    memcpy(ret, str + start, size);
    ret[size] = '\0';
    return ret;
}

char *substr_p(const char *str, const char *sp, const char *ep)
{
    size_t size;// = ep - sp;
    if (!ep) return NULL;
    size = ep - sp;
    if (size <= 0) return NULL;
    
    char *ret = (char *)malloc(sizeof(char) * size + 1);
    memcpy(ret, str, size);
    
    ret[size] = '\0';
    return ret;
}

char **str_split(const char *src, const char *delm)
{
    size_t slen = strlen(src);
    size_t delmlen = strlen(delm);
    char *str = (char *)malloc(sizeof(char) * slen + 1);
    assert(str);
    memset(str, 0, sizeof(char) * slen);
    memcpy(str, src, slen);
    int i = 0;
    int c = 0;
    for (; i <  slen; ++i) {
        if (memcmp(str+i, delm, delmlen) == 0) {
            ++c;
        }
    }
    
    if (c == 0) {
        return NULL;
    }
    
    char **res = (char **)malloc(sizeof(char*) * c);
    res[0] = str;
    int j;
    for (j=1, i = 0; i < slen; i++) {
        if (memcmp(str+i, delm, delmlen) == 0) {
            res[j-1][i] = '\0';
            res[j++] = str + i + delmlen;
        }
    }
    *(str + i) = '\0';
    return res;
}

int strpos(const char *str, const char *find)
{
    size_t findlen = strlen(find);
    size_t str_len = strlen(str);
    
    int i = 0;
    do {
        if (memcmp(str + i, find, findlen) == 0) {
            return i;
        }
    } while (i++ < str_len);
    
    return -1;
}

void trim_string(char *buf, const char *find)
{
    assert(buf && find);
    
    int pos = strpos(buf, find);
    if (pos == -1) {
        return;
    }
    
    size_t len = strlen(buf);
    size_t findlen = strlen(find);
    int i = 0;
    while ((pos + findlen) <= len) {
        buf[pos] = buf[pos + findlen];
        i++;pos++;
    }
}

void trim_string_end(char *buf, const char *find)
{
    assert(buf && find);
    
    int pos = strpos(buf, find);
    if (pos == -1) {
        return;
    }
    
    size_t len = strlen(buf);
    size_t findlen = strlen(find);
    int i = 0;
    while (i <= findlen) {
        buf[pos] = '\0';
        i++;pos++;
    }
    while (pos < len) buf[pos++] = '\0';
}

void str_replace(char *buf, const char *search, const char *replace)
{
    assert(buf && search && replace);
    
    int pos = strpos(buf, search);
    if (pos == -1) return;
    
    size_t slen = strlen(search);
    size_t rlen = strlen(replace);
    assert(slen == rlen);
    
    int i = 0;
    while (i < slen) {
        buf[pos + i] = replace[i];
        i++;
    }
}

void str_replace_c(char *buf, const char *search, const char replace)
{
    assert(buf && search && replace);
    int pos = strpos(buf, search);
    if (pos == -1) return;
    
    size_t slen = strlen(search);
    int i = 0;
    while (i < slen) {
        buf[pos + i] = replace;
        i++;
    }
}


size_t                          /* O - Length of string */
str_lcat(char       *dst,       /* O - Destination string */
         const char  *src,       /* I - Source string */
         size_t      size)       /* I - Size of destination string buffer */
{
    size_t    srclen;           /* Length of source string */
    size_t    dstlen;           /* Length of destination string */
    /*
     * Figure out how much room is left...
     */
    
    dstlen = strlen(dst);
    size   -= dstlen + 1;
    
    if (!size)
        return (dstlen);          /* No room, return immediately... */
    
    /*
     * Figure out how much room is needed...
     */
    
    srclen = strlen(src);
    /*
     * Copy the appropriate amount...
     */
    
    if (srclen > size)
        srclen = size;
    
    memcpy(dst + dstlen, src, srclen);
    dst[dstlen + srclen] = '\0';
    
    return (dstlen + srclen);
}

size_t                          /* O - Length of string */
str_rcat(char       *dst,       /* O - Destination string */
         const char  *src,       /* I - Source string */
         size_t      size)       /* I - Size of destination string buffer */
{
    size_t    srclen;           /* Length of source string */
    size_t    dstlen;           /* Length of destination string */
    /*
     * Figure out how much room is left...
     */
    
    dstlen = strlen(dst);
    size   -= dstlen + 1;
    
    if (!size)
        return (dstlen);          /* No room, return immediately... */
    
    /*
     * Figure out how much room is needed...
     */
    
    srclen = strlen(src);
    /*
     * Copy the appropriate amount...
     */
    
    if (srclen > size)
        srclen = size;
    
    char tmp[1024] = {0};
    memcpy(tmp, src, srclen);
    str_lcat(tmp, dst, sizeof(tmp));
    
    memcpy(dst, tmp, size);
    dst[size - 1] = '\0';
    return (dstlen + srclen);
}

