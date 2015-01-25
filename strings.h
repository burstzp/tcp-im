//
//  strings.h
//  pushim
//
//  Created by 易国磐 on 15-1-24.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__strings__
#define __pushim__strings__

#include <stdio.h>

int is_empty(const char *src);
char *ltrim(const char *str);
char *rtrim(char *str);
char *trim(const char *str);
char *substr(const char *str, size_t start, size_t len);
char **str_split(const char *src, const char *delm);
char *substr_p(const char *str, const char *sp, const char *ep);
int strpos(const char *str, const char *find);
void trim_string(char *buf, const char *find);
void trim_string_end(char *buf, const char *find);

void str_replace(char *buf, const char *search, const char *replace);
void str_replace_c(char *buf, const char *search, const char replace);

size_t str_lcat(char *dst, const char *src, size_t size);
size_t str_rcat(char *dst, const char *src, size_t size);

#endif /* defined(__pushim__strings__) */
