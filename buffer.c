#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "buffer.h"

buffer * buffer_init(void)
{
    buffer *buf = malloc(sizeof(*buf));
    bzero(buf, sizeof(*buf));
    assert(buf);
    
    buf->ptr = NULL;
    buf->size = 0;
    buf->used = 0;
    
    return buf;
}

void buffer_reset(buffer * buf)
{
    if (!buf) return;
    
    if(buf->size > BUFFER_MAX_REUSE_SIZE)
        {
        free(buf->ptr);
        buf->ptr = NULL;
        buf->size = 0;
        }
    
    buf->used = 0;
}

void buffer_free(buffer * buf)
{
    if (!buf) return;
    
    if (buf->ptr) free (buf->ptr);
    
    free (buf);
}

#define BUFFER_PIECE_SIZE       64
int buffer_prepare_copy (buffer * buf, int size)
{
    if (!buf || size <= 0) return -1;
    
    if ((buf->size == 0) || buf->size < size)
        {
        if (buf->size) free (buf->ptr);
        
        buf->size = size;
        /* allocate a multipy of BUFFER_PIECE_SIZE */
        if(buf->size % BUFFER_PIECE_SIZE)
            buf->size += BUFFER_PIECE_SIZE - (buf->size % BUFFER_PIECE_SIZE);
        
        buf->ptr = malloc (buf->size);
        assert (buf->ptr);
        }
    buf->used = 0;
    
    return 0;
}

int buffer_copy_string(buffer *buf,const char *str)
{
    if(!buf || !str) return -1;
    
    int len = strlen(str);
    
    buffer_prepare_copy(buf,len + 1);
    memcpy(buf->ptr,str,len + 1);                           /* copy str and ending '\0' */
    buf->used = len;                                        /* not with ending '\0' */
    
    return 0;
}

int buffer_copy_str_len(buffer *buf,const char *str,int len)
{
    if(!buf || !str || len <= 0) return -1;
    
    buffer_prepare_copy(buf,len + 1);
    memcpy(buf->ptr,str,len);
    buf->used = len;
    buf->ptr[buf->used] = '\0';
    
    return 0;
}

int buffer_prepare_append (buffer * buf, int size)
{
    if (!buf || size <= 0) return -1;
    
    if (buf->size == 0)
        {
        buf->size = size;
        
        /* allocate a multipy of BUFFER_PIECE_SIZE */
        if(buf->size % BUFFER_PIECE_SIZE)
            buf->size += BUFFER_PIECE_SIZE - (buf->size % BUFFER_PIECE_SIZE);
        
        buf->ptr = malloc (buf->size);
        assert (buf->ptr);
        buf->used = 0;
        }
    else if (buf->used + size > buf->size)
        {
        buf->size += size;
        
        /* allocate a multipy of BUFFER_PIECE_SIZE */
        if(buf->size % BUFFER_PIECE_SIZE)
            buf->size += BUFFER_PIECE_SIZE - (buf->size % BUFFER_PIECE_SIZE);
        
        buf->ptr = (char *) realloc (buf->ptr, buf->size);
        assert (buf->ptr);
        }
    return 0;
}

int buffer_append_string (buffer * buf, const char *str)
{
    if (!buf || !str) return -1;
    
    int len = strlen (str);
    
    buffer_prepare_append (buf, len + 1);
    
    memcpy (buf->ptr + buf->used, str, len + 1);            /* copy str and ending '\0' */
    buf->used += len;
    
    return 0;
}

int buffer_append_str_len (buffer * buf, const char *str, int len)
{
    if (!buf || !str || len <= 0) return -1;
    
    buffer_prepare_append (buf, len + 1);
    
    memcpy(buf->ptr + buf->used, str, len);
    buf->used += len;
    buf->ptr[buf->used] = '\0';
    
    return 0;
}

int itostr (char *buf, int n)
{
    if (!buf) return -1;
    
    int len = 1;
    char swap;
    char *end;
    
    if (n < 0)
        {
        len++;
        *(buf++) = '-';
        n = -n;
        }
    
    end = buf;
    while (n > 9)
        {
        *(end++) = '0' + (n % 10);
        n /= 10;
        }
    *end = '0' + n;
    *(end + 1) = '\0';
    len += end - buf;
    
    while (buf < end)
        {
        swap = *buf;
        *buf = *end;
        *end = swap;
        
        buf++;
        end--;
        }
    
    return len;
}

int buffer_append_int (buffer * buf, int n)
{
    if (!buf) return -1;
    
    buffer_prepare_append (buf, 32);
    
    buf->used += itostr (buf->ptr + buf->used, n);
    
    return 0;
}

int buffer_cmp(buffer *bs,buffer *bd)
{
    if(!bs || !bd) return -1;
    
    if(bs->used != bd->used) return -1;
    return strncmp(bs->ptr,bd->ptr,bs->used);
}

static char *strcpy_overlap(char *dst,const char *src)
{
    char *dp = dst;
    const char *sp = src;
    
    if(src > dst)
        {
        while(*src) *dst++ = *src++;
        *dst = '\0';
        }
    else if(src < dst)
        {
        while(*src)
            {
            src++;
            dst++;
            }
        while(src >= sp) *dst-- = *src--;
        }
    return dp;
}
int buffer_replace(buffer *b,const char *old,const char *new)
{
    if(!b || !old || !new) return -1;
    char *sp;
    int oldlen = strlen(old),newlen = strlen(new);
    
    sp = strstr(b->ptr,old);
    if(!sp) return -1;
    
    if(b->used + newlen - oldlen >= b->size)
        {
        b->size += newlen - oldlen;
        if(b->size % BUFFER_PIECE_SIZE)
            b->size += BUFFER_PIECE_SIZE - (b->size % BUFFER_PIECE_SIZE);
        
        b->ptr = (char *) realloc (b->ptr,b->size);
        assert(b->ptr);
        
        buffer_replace(b,old,new);
        }
    else
        {
        strcpy_overlap(sp + newlen,sp + oldlen);                //strcpy cann't deal with overlap copy...poor.
        strncpy(sp,new,newlen);
        b->used += newlen - oldlen;
        }
    return 0;
}

int buffer_replace_all(buffer *b,const char *old,const char *new)	//e...
{
    while(buffer_replace(b,old,new) == 0);
    return 0;
}

void buffer_shortcut(buffer *buf,int maxlen)
{
    if(!buf || maxlen <= 0) return;
    
    if(buf->used > maxlen)
        {
        buf->used = maxlen;
        if((unsigned char)buf->ptr[buf->used - 1] <= 0xa0) buf->used--;         /* for chinese character */
        buf->ptr[buf->used] = '\0';
        
        if(buf->used >= 1) buf->ptr[buf->used - 1] = '.';
        if(buf->used >= 2) buf->ptr[buf->used - 2] = '.';
        if(buf->used >= 3) buf->ptr[buf->used - 3] = '.';
        }
}

/* trim begining and ending ' ','\t' */
void buffer_trim(buffer *buf)
{
    if(!buf) return;
    char *sp,*ep;
    
    sp = buf->ptr;
    while(*sp != '\0' && (*sp == ' ' || *sp == '\t')) ++sp;
    ep = buf->ptr + buf->used - 1;
    while(ep >= sp && (*ep == ' ' || *ep == '\t')) --ep;
    
    if(sp != buf->ptr)
        memmove(buf->ptr,sp,ep - sp + 1);
    buf->used = ep - sp + 1;
    buf->ptr[buf->used] = '\0';
}

int buffer_is_equal_string(buffer *buf,const char *str,int len)
{
    if(!buf || !str) return 0;
    if(buf->used != len) return 0;
    if(len == 0) return 1;
    
    return (strncmp(buf->ptr,str,len) == 0);
}

array_buffer *array_buffer_init(int initsiz)
{
    if(initsiz < 0) return NULL;
    
    array_buffer *ab = malloc(sizeof(*ab));
    assert(ab);
    int i;
    
    ab->ptr = malloc(sizeof(*ab->ptr) * initsiz);
    assert(ab->ptr);
    
    for(i = 0;i < initsiz;i++) ab->ptr[i] = buffer_init();		//when big,change to buffer_init_n
    
    ab->size = initsiz;
    ab->used = 0;
    
    return ab;
}

void array_buffer_free(array_buffer *ab)
{
    if(!ab) return;
    int i;
    
    for(i = 0;i < ab->size;i++) buffer_free(ab->ptr[i]);
    
    free(ab->ptr);
    free(ab);
}

int array_buffer_find_string(array_buffer *ab,const char *str,int len)
{
    if(!ab || !str || len <= 0) return -1;
    int i;
    
    for(i = 0;i < ab->used;i++)
        {
        if(buffer_is_equal_string(ab->ptr[i],str,len)) 
            return i;
        }
    
    return -1;
}

buffer *array_buffer_get_unused_ele(array_buffer *ab)
{
    assert(ab);
    
    if(ab->used >= ab->size)
        {
        int i;
        ab->size += 4;
        ab->ptr = (buffer **)realloc(ab->ptr,sizeof(*ab->ptr) * ab->size);
        for(i = ab->used;i < ab->size;i++) ab->ptr[i] = buffer_init();
        }
    return ab->ptr[ab->used++];
}
