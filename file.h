//
//  file.h
//  pushim
//
//  Created by 易国磐 on 15-1-24.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#ifndef __pushim__file__
#define __pushim__file__

#include "buffer.h"

int is_file(const char *filename);
int file_exists(const char *filename);
int is_dir(const char *filename);
buffer *file_get_contents(const char *filename);
int file_put_contents(const char *filename, const char *content, const char *mode);
int mk_dir(const char *path);
int mk_dirs(const char *dir);

#endif /* defined(__pushim__file__) */
