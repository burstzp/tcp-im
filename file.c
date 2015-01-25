#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#include<stdlib.h>

#include "file.h"
#include "buffer.h"

int is_file(const char *filename)
{
    struct stat st;
    return stat(filename, &st);
}

int file_exists(const char *filename)
{
    struct stat st;
    return stat(filename, &st);
}

int is_dir(const char *filename)
{
    struct stat st;
    if (stat(filename,&st) == -1) {
        return -1;
    } else {
        if (st.st_mode & S_IFDIR) {
            return 0;
        } else {
            return -1;
        }
    }
}

buffer *file_get_contents(const char *filename)
{
    buffer *bufz = buffer_init();
    char buf[4096];
    FILE *fp = fopen(filename, "rb");
    assert(fp);
    while(!feof(fp) && !ferror(fp)){
        int n = fread(buf, 1, sizeof(buf), fp);
        if(n > 0){
            bufz->size += n;
            BUFFER_APPEND_CONST_STRING(bufz, buf);
        }
    }
    
    fclose(fp);
    return bufz;
}

int file_put_contents(const char *filename, const char *content, const char *mode)
{
    FILE *fp;
    if (mode) {
        fp = fopen(filename, mode);
    } else {
        fp = fopen(filename, "wb");
 
    }
    assert(fp);
    
    int len = strlen(content);
    int siz = fwrite(content, 1, len, fp);
    fclose(fp);
    
    return siz == len? siz : -1;
}

int mk_dir(const char *path)
{
    if (is_dir(path) == -1) return -1;
    
    int status = mkdir(path, 0755);
    if (status == -1) {
        return -1;
    }
    
    return 0;
}

int mk_dirs(const char *dir)
{
    char tmp[256];
    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, dir, sizeof(tmp));
    int i, len = strlen(tmp);
    if (tmp[len-1] != '/')
        strncat(tmp, "/", 1);
    
    len = strlen(tmp);
    for (i = 1; i < len; i++) {
        if (tmp[i] != '/') {
            continue;
        } else {
            tmp[i] = '\0';
            if (access(tmp, 0) != 0) {
                if (mkdir(tmp, 0755) == -1) {
                    fprintf(stderr, "mk_dirs: tmp=%s\n", tmp);
                    return -1;
                }
            }
            tmp[i] = '/';
        }
    }
    
    return 0;
} 

