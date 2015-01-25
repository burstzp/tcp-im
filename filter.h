//
//  filter.h
//  pushim
//
//  Created by 易国磐 on 15-1-25.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__filter__
#define __pushim__filter__

#include <stdio.h>

void get_login_name(const char *buf, char *login, const char *delim);
void get_talk_msg(const char *buf, char *talkto, char *msg, const char *delim);

#endif /* defined(__pushim__filter__) */
