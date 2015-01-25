//
//  httputil.h
//  pushim
//
//  Created by 易国磐 on 15-1-25.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__httputil__
#define __pushim__httputil__

#include <stdio.h>
#include <curl/curl.h>

#include "buffer.h"

CURL *curl;
void curl_init();
void curl_destroy();
int curl_get(const char *url, char *buffer);

#endif /* defined(__pushim__httputil__) */
