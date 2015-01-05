//
//  main.c
//  StudentManager
//
//  Created by 易国磐 on 15-1-5.
//  Copyright (c) 2015年 易国磐. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct student *stu;
struct student
{
    unsigned int uid;
    unsigned int no;
    char name[50];
    uint8_t sex;
    stu prev;
    stu next;
};

void insert(stu *node, stu data)
{
    (*node)->next   = data;
    data->prev      = (*node);
    (*node)         = data;
}

void travseval(stu node, (* visit)(stu nod))
{
    stu data = node;
    for (; data && data->prev; data = data->prev) {
        visit(data);
    }
}

void dump(stu node)
{
    printf("addr = %x, uid = %d, name = %s, no = %d, sex = %d\n",
           node,
           node->uid,
           node->name,
           node->no,
           node->sex
         );
}

stu stu_node_make(unsigned int uid, unsigned int no, const char *name, uint8_t sex)
{
    stu node = (stu)malloc(sizeof(struct student));
    if (!node) {
        return NULL;
    }
    
    node->uid   = uid;
    node->no    = no;
    node->sex   = sex;
    node->prev  = NULL;
    node->next  = NULL;
    strcpy(node->name, name);

    return node;
}

stu search(stu node, unsigned int uid)
{
    stu data = node;
    for (; data; data = data->prev) {
        if (data->uid == uid)
            return data;
    }

    return NULL;
}

int delete(stu node, unsigned int uid)
{
    stu data = search(node, uid);
    if (!data)
        return -1;

    stu t1 = data->next;
    stu t2 = data->prev;
    t1->prev = t2;
    t2->next = t1;
    free(data);
    return 0;
}

int main(int argc, const char * argv[]) {
    
    stu node = (stu)malloc(sizeof(struct student));
    
    insert(&node, stu_node_make(100000, 1, "yiguopan", 1));
    insert(&node, stu_node_make(100001, 2, "liudehua", 1));
    insert(&node, stu_node_make(100002, 2, "liudehua", 1));

    delete(node, 100001);
    
    stu find = search(node, 100002);
    dump(find);
    travseval(node, dump);
    return 0;
}
