//
//  hashtable.c
//  pushim
//
//  Created by 易国磐 on 15-1-24.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#include "slabs.h"
#include "hashtable.h"

#define MALLOC_CHAR(z, len)   		(z) = (char *)malloc(sizeof(char) * len + 1)
#define APPEN_CHAR_ZERO(z, len) 	*((z) + len) = '\0'
#define MALLOC_BUCKET_P(z)          (z) = (Buckets *)malloc(sizeof(Buckets))
#define MALLOC_BUCKET_PP(z)         (z) = (Buckets **)malloc(sizeof(Buckets *))
#define MALLOC_HVAL(z)              (z) = (Hval*)malloc(sizeof(Hval));

#define MEMCPY_STR(z, str, str_len) do {                            \
MALLOC_CHAR((z), str_len);                                      \
memcpy((z), str, str_len);                                      \
APPEN_CHAR_ZERO((z), str_len);                                  \
} while(0);

#define INSERT_BUCKET(element, list) do {                           \
(element)->right = (list);                                      \
(element)->left = NULL;                                         \
if ((element)->right) {                                         \
(element)->right->left = (element);                         \
}                                                               \
(list) = (element);                                             \
} while(0);

#define DELETE_BUCKET(element, list) do {                           \
if ((element)->right) {                                         \
(element)->right->left = (element)->left;                   \
} else {                                                        \
(list) = (element)->left;                                   \
}                                                               \
if ((element)->left) {                                          \
(element)->left->right = (element)->right;                  \
} else {                                                        \
(list) = (element)->right;                                  \
}                                                               \
} while(0);

#define INSERT_LIST(element, ht) do {                               \
(element)->list_right = (ht)->tail;                             \
(element)->list_left  = NULL;                                   \
if ((element)->list_right != NULL) {                            \
(element)->list_right->list_left = (element);               \
}                                                               \
if (!(ht)->head) {                                              \
(ht)->head = element;                                       \
}                                                               \
(ht)->tail = (element);                                         \
} while(0);

#define DELETE_LIST(element, ht) do {                               \
if ((element)->list_left) {                                     \
(element)->list_left->list_right = (element)->list_right;    \
} else {                                                        \
(ht)->tail = (element)->list_right;                         \
}                                                               \
if ((element)->list_right) {                                    \
(element)->list_right->list_left = (element)->list_left;    \
} else {                                                        \
(ht)->head = (element)->list_left;                          \
}                                                               \
} while(0);

ulong hash_func(const char *arKey) {
    register ulong hash = 5381;
    int      nKeyLength = strlen(arKey);
    
    for (; nKeyLength >= 8; nKeyLength -= 8) {
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
        hash = ((hash << 5) + hash) + *arKey++;
    }
    switch (nKeyLength) {
        case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
        case 1: hash = ((hash << 5) + hash) + *arKey++; break;
        case 0: break;
        default:
            break;
    }
    return hash;
}

HashTable *_create_hashtable(uint size) {
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    assert(ht);
    ht->tableSize = size;
    ht->tableMask = size - 1;
    ht->arrBuckets = (Buckets **)malloc(sizeof(Buckets *) * size);
    ht->numberOffElements = 0;
    return ht;
}

int hash_find(HashTable *ht, const char *key, Hval **ret) {
    ulong h = hash_func(key);
    int nIndex = h & ht->tableMask;
    Buckets *p = ht->arrBuckets[nIndex];
    
    while (p != NULL) {
        if (!strcmp(p->key, key)) {
            *ret = p->val;
            return 0;
        }
        p = p->right;
    }
    
    return -1;
}

ulong numberOfElements(HashTable *ht) {
    return ht->numberOffElements;
}

int hash_add(HashTable *ht, const char *key, Hval *val) {
    
    Hval *find;
    if (0 == hash_find(ht, key, &find)) {
        return 0;
    }
    
    Buckets *p;
    MALLOC_BUCKET_P(p);
    assert(p);
    MEMCPY_STR(p->key, key, strlen(key));
    p->val = val;
    
    ulong h = hash_func(key);
    int nIndex = h & ht->tableMask;
    INSERT_BUCKET(p, ht->arrBuckets[nIndex]);
    INSERT_LIST(p, ht);
    
    ht->numberOffElements++;
    return 0;
}

int hash_del(HashTable *ht, char *key) {
    ulong h = hash_func(key);
    int nIndex = h & ht->tableMask;
    Buckets *p = ht->arrBuckets[nIndex];
    while (p != NULL) {
        if (!strcmp(p->key, key)) {
            DELETE_BUCKET(p, ht->arrBuckets[nIndex]);
            DELETE_LIST(p, ht);
            slabs_free(p->key, strlen(key)+1);
            slabs_free(p->val->username, p->val->size + 1);
            slabs_free(p, sizeof(Hval));
            return 0;
        }
        p = p->right;
    }
    
    return -1;
}

#define reset(ht) ht->cur = ht->head
#define istail(ht) ht->cur != NULL
#define next(ht)  ht->cur = ht->cur->list_left
#define ivalue(ht) ht->cur->val->lval
#define key(ht)   ht->cur->key
#define value(ht) ht->cur->val->str.val

inline Hval *hval_init()
{
    Hval *hval = malloc(sizeof(*hval));
    assert(hval);
    bzero(hval, sizeof(*hval));
    hval->size = 0;
    hval->uid = 0;
    hval->sock = 0;
    return hval;
}

inline Hval *make_hval(int sock, uint uid, char *username)
{
    Hval *hval = hval_init();
    hval->uid = uid;
    hval->size = strlen(username);
    hval->sock = sock;
    memcpy(hval->username, username, hval->size);
    return hval;
}

//inline void hash_dump_kvint(HashTable *ht) {
//    for (reset(ht);istail(ht);next(ht)) {
//        printf("key = %d, val = %d \n", key(ht), ivalue(ht));
//    }
//}
//
//inline void hash_dump_kvstr(HashTable *ht) {
//    for (reset(ht);istail(ht);next(ht)) {
//        printf("key = %s, val = %s\n", key(ht), ht->cur->val->str.val);
//    }
//}
//
//inline void hash_dump_ksvi(HashTable *ht) {
//    for (reset(ht);istail(ht);next(ht)) {
//        printf("key = %s, val = %d \n", key(ht), ivalue(ht));
//    }
//}
//
//inline void hash_dump_kivs(HashTable *ht) {
//    for (reset(ht);istail(ht);next(ht)) {
//        printf("key = %d, val = %s \n", key(ht), value(ht));
//    }
//}
//
//inline void hash_dump(HashTable *ht, fp p) {
//    p(ht);
//}
//
//inline Hval *long_hval(long val) {
//    Hval *hval;
//    MALLOC_HVAL(hval);
//    if (hval == NULL) return NULL;
//    hval->lval = val;
//    return hval;
//}
//
//inline Hval *double_hval(double val) {
//    Hval *hval;
//    MALLOC_HVAL(hval);
//    if (hval == NULL) return NULL;
//    hval->dval = val;
//    return hval;
//}
//
//inline Hval *str_hval(const char *val) {
//    Hval *hval;
//    MALLOC_HVAL(hval);
//    if (hval == NULL) return NULL;
//    int len = strlen(val);
//    MEMCPY_STR(hval->str.val, val, len);
//    hval->str.len = len;
//    return hval;
//}
