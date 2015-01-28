
//
//  response.h
//  pushim
//
//  Created by 易国磐 on 15-1-26.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__response__
#define __pushim__response__

#include <stdio.h>

typedef struct response
{
    char *body;
    size_t blen;
    size_t size;
//    buffer *buffer;
}response_t;

void response_free(response_t *response);

#endif /* defined(__pushim__response__) */
