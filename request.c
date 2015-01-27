//
//  request.c
//  pushim
//
//  Created by 易国磐 on 15-1-26.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#include "request.h"

void request_free(request_t *request)
{
    if (request->body) {
        //free(request->body);
    }
}
