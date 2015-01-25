//
//  slabs.h
//  pushim
//
//  Created by 易国磐 on 15-1-24.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__slabs__
#define __pushim__slabs__

unsigned int slabs_clsid(unsigned int size);
void slabs_init(unsigned int limit);
int slabs_newslab(unsigned int id);
void *slabs_alloc(unsigned int size);
void slabs_free(void *ptr, unsigned int size);
char* slabs_stats(int *buflen);

#endif /* defined(__pushim__slabs__) */
