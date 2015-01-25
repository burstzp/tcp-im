//
//  httputil.c
//  pushim
//
//  Created by 易国磐 on 15-1-25.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#include "httputil.h"
#include <assert.h>
#include <strings.h>

void curl_init()
{
    assert(curl == NULL);
    curl = curl_easy_init();
}

void curl_destroy()
{
    assert(curl);
    curl_easy_cleanup(curl);
}

size_t curl_receive(void *ptr, size_t size, size_t nmemb, void *stream)
{
    strcat(stream, (char *)ptr);
    return size*nmemb;
}

int curl_get(const char *url, char *buffer)
{
    assert(url && curl);
    curl_easy_setopt(curl, CURLOPT_URL, url); //设置下载地址
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);//设置超时时间
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive);//设置写数据的函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);//设置写数据的变量
    
    int res = curl_easy_perform(curl);
    if(CURLE_OK != res) return -1;
    
    return 0;
}

int curl_post(const char *url, const char *post_field, char *buffer)
{
    assert(url && curl);
    
    curl_easy_setopt(curl, CURLOPT_URL, url); //设置下载地址
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_field);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, sizeof(post_field));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);//设置超时时间
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_receive);//设置写数据的函数
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);//设置写数据的变量
    
    int res = curl_easy_perform(curl);
    if(CURLE_OK != res) return -1;
    
    return 0;
}