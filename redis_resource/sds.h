//
//  sds.h
//  redis_resource
//
//  Created by pengge on 17/3/13.
//  Copyright © 2017年 pengge. All rights reserved.
//

#ifndef sds_h
#define sds_h

#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <assert.h>

#define SDS_MAX_PREALLOC  1024*1024
typedef char *sds;

//redis 自定义字符串结构
struct sdshdr {
    int len;
    int free;
    char buf[];
};
//获取sds s 实际保存的字符串的长度
static inline size_t sdslen(const sds s) {
    //这里有几点注意点。sizeof(struct sdshdr)的长度为8，具体原因查看https://www.douban.com/note/237669531/
    /**
        1.sdshdr 中的char buf[]为 flexible array member 在计算结构体大小的时候不计入在内，
        则 sizeof(struct sdshdr) = sizeof(unsigned int) + sizeof(unsigned int)
        2.s-sizeof(struct sdshdr) 表示s 向前位移8个单位长度，这个时候的地址为len的首地址，转化为结构体sdshdr 的时候，则为该结构体的首地址,则 sh->len就可以获取该结构体的长度了
    **/
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    return sh->len;
}
//获取sds 可用空间的长度
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    return sh->free;
}

sds sdsnewlen(const char *init ,size_t len);
sds sdsnew(const char *init);
sds sdsempty();
sds sdsdup(const sds s);
void sdsfree(sds s);
sds sdsMakeRoomFor(sds s, size_t addlen);
sds sdsRemoveFreeSpace(sds s);
size_t sdsAllocSize(sds s);
void sdsIncrLen(sds s, int incr);
sds sdsgrowzero(sds s, size_t len);
#endif /* sds_h */
