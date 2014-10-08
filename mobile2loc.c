/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: yiguopan<yiguopan@126.com>                                                             |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __c
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __c
}
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_mobile2loc.h"

#define INDEX_LEN_READ_NUM 4
#define PHONE_NUM          11 

ZEND_DECLARE_MODULE_GLOBALS(mobile2loc)

char machine_little_endian;
static char *item   = NULL;
static int filesize = 0;

#define index_get_all_len(offset, index) do {           \
    memcpy(&(offset), (index), (INDEX_LEN_READ_NUM));   \
    if (!machine_little_endian) {                       \
        offset = lb_reverse(offset);                    \
    }                                                   \
    if (!offset) RETURN_FALSE;                          \
} while(0)

#define index_get_first_index_len(first_index_len, index) do {                  \
    memcpy(&first_index_len, index + INDEX_LEN_READ_NUM, INDEX_LEN_READ_NUM);   \
    if (!machine_little_endian) {                                               \
        first_index_len = lb_reverse(first_index_len);                          \
    }                                                                           \
    if (!first_index_len) RETURN_FALSE;                                         \
} while(0)

#define index_unpack(val, index, len) do {              \
    memcpy(&val, (index), len);                         \
    if (!machine_little_endian) {                       \
       val = lb_reverse(val);                           \
    }                                                   \
} while(0)

#define second_index_len(second_index_len, offset, first_index_len) do { \
   second_index_len = offset - INDEX_LEN_READ_NUM * 2 - first_index_len; \
} while(0)

#define move_pointer_to_second_index(tmp, second_index_offset, first_index_len) do {\
    tmp = item + INDEX_LEN_READ_NUM * 2 + first_index_len + second_index_offset;    \
}while(0)

#define move_pointer_to_first_index(tmp) do {                                       \
    tmp = item + INDEX_LEN_READ_NUM * 2;                                            \
} while(0)
/* {{{ mobile2loc_functions[]
 *
 * Every user visible function must have an entry in mobile2loc_functions[].
 */
const zend_function_entry mobile2loc_functions[] = {
    PHP_FE(mobile2loc, NULL)
    PHP_FE_END  /* Must be the last line in mobile2loc_functions[] */
};
/* }}} */

/* {{{ mobile2loc_module_entry
 */
zend_module_entry mobile2loc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "mobile2loc",
    mobile2loc_functions,
    PHP_MINIT(mobile2loc),
    PHP_MSHUTDOWN(mobile2loc),
    PHP_RINIT(mobile2loc),      /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(mobile2loc),  /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(mobile2loc),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_MOBILE2LOC_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MOBILE2LOC
ZEND_GET_MODULE(mobile2loc)
#endif

PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("mobile2loc.filename", "", PHP_INI_ALL, OnUpdateString, filename, zend_mobile2loc_globals, mobile2loc_globals)
PHP_INI_END()

/*{{{*/
static int lb_reverse(int a){
    union {
        int i;
        char c[4];
    }u, r;
    u.i = a;
    r.c[0] = u.c[3];
    r.c[1] = u.c[2];
    r.c[2] = u.c[1];
    r.c[3] = u.c[0];
    return r.i;
}
/*}}}*/
/*{{{*/
/*{{{*/
static char *substr(const char *str, int position, int length) {
    char *pointer = (char *)emalloc(length + 1);
    strncpy(pointer, str + position, length);
    *(pointer + length) = '\0';
    return pointer;
}
/*}}}*/
/* {{{ 
 */
static int mobile2loc_init_mem(TSRMLS_CC) {
    if (item != MAP_FAILED && filesize) {
        return 1;
    }
    
    struct stat sb;
    int fp = open(MOBILE2LOC_G(filename), O_RDONLY);
    if (fp == -1) {
        return 0;
    }
    fstat(fp, &sb);
    if (sb.st_size == 0) {
        return 0;
    }
    filesize = sb.st_size;
    item = (char *)mmap(NULL, filesize, PROT_READ, MAP_SHARED, fp, 0);
    close(fp);
    return item == MAP_FAILED ? 0 : 1;
}
static int str2int(const char *str, int length) {
    char *tmp = substr(str, 0, length);                                       
    int rst = atoi(tmp);                                      
    efree(tmp);                                             
    return rst;
}
/*}}}*/
/*{{{*/
static int get_second_index_offset(const char *src, int index_len, int search) {
    int i, key;
    uint rst = 0;
    for (i = 0; i < index_len; i += 8) {
        index_unpack(key, src + i, 4);
        if (key == search) {
            index_unpack(rst, src + i + 4, 4);
            return rst;
        }
    }
    return -1;
}
/*}}}*/
/*{{{*/
PHP_FUNCTION(mobile2loc) {
    char *phone;
    int phone_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &phone, &phone_len) == FAILURE) {
        RETURN_FALSE;
    }
    if (phone_len != PHONE_NUM) RETURN_FALSE;
    
    int thetop7 = str2int(phone, 7);
    int thetop4 = str2int(phone, 4);
    
    char *key = (char *)&thetop7;
    if (!mobile2loc_init_mem(TSRMLS_CC)) RETURN_FALSE;
    
    int all_index_len, first_index_len;
    index_get_all_len(all_index_len, item);
    index_get_first_index_len(first_index_len, item);

    char *tmp = NULL;
    move_pointer_to_first_index(tmp);
    int second_index_offset = -1;
    if (-1 == (second_index_offset = get_second_index_offset(tmp, first_index_len, thetop4))) {
        RETURN_FALSE;
    }
    
    int second_index_len;
    second_index_len(second_index_len, all_index_len, first_index_len);
    move_pointer_to_second_index(tmp, second_index_offset, first_index_len);

    int i                   = 0;
    uint data_offset        = 0;
    unsigned char data_len  = 0;
    for (; i < second_index_len; i += 9) {
        if (memcmp(tmp + i, key, 4) == 0) {
            index_unpack(data_offset, tmp + i + 4, 4);
            data_len = *(tmp + i + 8);
            char *str = substr(item + (all_index_len + data_offset), 0, data_len);
            tmp = NULL;
            RETURN_STRINGL(str, data_len, 0);
        }
    }
    
    RETURN_FALSE; 
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(mobile2loc)
{
    REGISTER_INI_ENTRIES();
    int machine_endian_check = 1;
    machine_little_endian = ((char *)&machine_endian_check)[0];
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(mobile2loc)
{
    UNREGISTER_INI_ENTRIES();
    if (item && filesize) {
        munmap((void *)item, filesize);
    }
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(mobile2loc)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(mobile2loc)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(mobile2loc)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "mobile2loc support", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
