//
//  buffer.h
//  pushim
//
//  Created by 易国磐 on 15-1-24.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__buffer__
#define __pushim__buffer__

/**
 * A '\0' will be appended to the end of the string,but it is not calc-ed to 'used' (strlen)
 */
typedef struct
{
    char    *ptr;
    int     size;		//->u_int
    int     used;
}buffer;

#define	BUFFER_MAX_REUSE_SIZE	(4 * 1024)

buffer  *buffer_init(void);
void    buffer_reset(buffer *buf);
void    buffer_free(buffer *buf);

int buffer_prepare_copy(buffer *buf,int size);
int buffer_prepare_append(buffer *buf,int size);
int buffer_copy_string(buffer *buf,const char *str);            //ping
int buffer_copy_str_len(buffer *buf,const char *str,int len);   //ping
int buffer_append_string(buffer *buf,const char *str);
int buffer_append_str_len(buffer *buf,const char *str,int len);
int buffer_append_int(buffer *buf,int n);
int buffer_replace(buffer *b,const char *old,const char *nw);
int buffer_replace_all(buffer *b,const char *old,const char *nw);
int buffer_cmp(buffer *bs,buffer *bd);
void buffer_shortcut(buffer *buf,int maxlen);
void buffer_trim(buffer *buf);
int buffer_is_equal_string(buffer *a,const char *str,int len);

#define	CONST_STR_LEN(x)	x,sizeof(x) - 1

#define	BUFFER_COPY_CONST_STRING(x,y)		\
buffer_copy_str_len(x,CONST_STR_LEN(y))

#define BUFFER_APPEND_CONST_STRING(x,y) 	\
buffer_append_str_len(x,CONST_STR_LEN(y))

#define	buffer_copy_from_buffer(dst,src)	\
buffer_copy_str_len(dst,(src)->ptr,(src)->used)

#define	buffer_append_from_buffer(dst,src)	\
buffer_append_str_len(dst,(src)->ptr,(src)->used)

#define	buffer_is_empty(b)			\
((b)->used == 0)

#define	buffer_clean(b)	do {			\
(b)->ptr = NULL;				\
(b)->size = 0;				\
(b)->used = 0;				\
}while(0)

typedef struct
{
    buffer	**ptr;
    int	size;
    int	used;
}array_buffer;

array_buffer *array_buffer_init(int initsz);
buffer	*array_buffer_get_unused_ele(array_buffer *ab);
int	array_buffer_find_string(array_buffer *ab,const char *str,int len);
void	array_buffer_free(array_buffer *ab);
#endif /* defined(__pushim__buffer__) */
