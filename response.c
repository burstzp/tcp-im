//
//  response.c
//  pushim
//
//  Created by 易国磐 on 15-1-26.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#include "response.h"

void response_free(response_t *response)
{
    if (response->body) {
        free(response->body);
    }
}

