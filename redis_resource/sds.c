//
//  sds.c
//  redis_resource
//
//  Created by pengge on 17/3/13.
//  Copyright © 2017年 pengge. All rights reserved.
//

#include "sds.h"
//根据给定的初始化字符串 init 和字符串长度 len
//返回一个新的字符串(相当于结构体中的数据sh->buf)
sds sdsnewlen(const char *init ,size_t len) {
    struct sdshdr *sh;
    if(init) {
        sh = malloc(sizeof(struct sdshdr) + len + 1);
    }else {
        sh = calloc(1, sizeof(struct sdshdr) + len +1);
    }
    sh->len = (int)len;
    sh->free = 0;
    //memcpy 将init复制给sh->buf，并拷贝len长度的字节空间，如果len 小于init的长度，则init 会被截取
    //例如  memcpy("aaa",2) ,则返回aa
    if(init && len)
        memcpy(sh->buf, init, len);
    //buf末尾加上\0
    sh->buf[len] = '\0';
    return (char *)sh->buf;
}

/*
 * 根据给定字符串 init ，创建一个包含同样字符串的 sds
 *
 * 参数
 *  init ：如果输入为 NULL ，那么创建一个空白 sds
 *         否则，新创建的 sds 中包含和 init 内容相同字符串
 *
 * 返回值
 *  sds ：创建成功返回 sdshdr 相对应的 sds
 *        创建失败返回 NULL
 *
 * 复杂度
 *  T = O(N)
 */
sds sdsnew(const char *init) {
    size_t len = (init == NULL ) ? 0:sizeof(init);
    return sdsnewlen(init, len);
}

/*
 * 创建并返回一个只保存了空字符串 "" 的 sds
 *
 * 返回值
 *  sds ：创建成功返回 sdshdr 相对应的 sds
 *        创建失败返回 NULL
 *
 * 复杂度
 *  T = O(1)
 */
sds sdsempty() {
    return sdsnewlen("", 0);
}

/*
 * 复制给定 sds 的副本
 *
 * 返回值
 *  sds ：创建成功返回输入 sds 的副本
 *        创建失败返回 NULL
 *
 * 复杂度
 *  T = O(N)
 */
sds sdsdup(const sds s) {
    return sdsnewlen(s, sdslen(s));
}

/*
  * 释放给定的 sds
  *
  * 复杂度
  *  T = O(N)
  */
void sdsfree(sds s) {
    if(s == NULL) return;
    free(s-sizeof(struct sdshdr));
}
/*
 * 在不释放 SDS 的字符串空间的情况下，
 * 重置 SDS 所保存的字符串为空字符串。
 *
 * 复杂度
 *  T = O(1)
 */
void sdsclear(sds s) {
    struct sdshdr *sh = (void *)(s - sizeof(struct sdshdr));
    sh->free += sh->len;
    sh->free += sh->len;
    sh->len = 0;
    // 将结束符放到最前面（相当于惰性地删除 buf 中的内容）
    sh->buf[0] = '\0';
}

/*
 * 对 sds 中 buf 的长度进行扩展，确保在函数执行之后，
 * buf 至少会有 addlen + 1 长度的空余空间
 * （额外的 1 字节是为 \0 准备的）
 *
 * 返回值
 *  sds ：扩展成功返回扩展后的 sds
 *        扩展失败返回 NULL
 *
 * 复杂度
 *  T = O(N)
 */
sds sdsMakeRoomFor(sds s, size_t addlen) {
    struct sdshdr *sh,*new_sh;
    sh = (void *)(s-sizeof(struct sdshdr));
    size_t len,new_len;
    size_t free = sdsavail(s);
    if(free > addlen)
        return s;
    len = sdslen(s);
    new_len = len + addlen;
    //这块拓展空间的策略是 如果小于1Mb，则每次申请为addlen的2倍，大于的话每次申请1Mb+addlen的长度
    if(new_len < SDS_MAX_PREALLOC) {
        new_len = 2*new_len;
    }else {
        new_len = new_len + SDS_MAX_PREALLOC;
    }
    //realloc 为拓展或者缩小原有的空间函数s
    new_sh = realloc(sh, sizeof(struct sdshdr)+new_len+1);
    if(new_sh == NULL) return NULL;
    //这块需要变更free的长度
    new_sh->free = (int)(new_len - len);
    return new_sh->buf;
}
/*
 * 回收 sds 中的空闲空间，
 * 回收不会对 sds 中保存的字符串内容做任何修改。
 *
 * 返回值
 *  sds ：内存调整后的 sds
 *
 * 复杂度
 *  T = O(N)
 */
/* Reallocate the sds string so that it has no free space at the end. The
 * contained string remains not altered, but next concatenation operations
 * will require a reallocation.
 *
 * After the call, the passed sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
sds sdsRemoveFreeSpace(sds s) {
    struct sdshdr *sh;
    sh = (void *)(s-sizeof(struct sdshdr));

    //使用realloc重新分配内存空间(在原来的len长度_1，将free设为0)
    sh = realloc(sh, sizeof(struct sdshdr)+(sh->len)+1);
    
    sh->free = 0;
    return sh->buf;
}

/*
 * 返回给定 sds 分配的内存字节数
 *
 * 复杂度
 *  T = O(1)
 */
/* Return the total size of the allocation of the specifed sds string,
 * including:
 * 1) The sds header before the pointer.
 * 2) The string.
 * 3) The free buffer at the end if any.
 * 4) The implicit null term.
 */
size_t sdsAllocSize(sds s) {
    struct sdshdr *sh = (void *)(s-sizeof(struct sdshdr));
    return sizeof(*sh)+sh->len+sh->free+1;
}

/* Increment the sds length and decrements the left free space at the
 * end of the string according to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * 根据 incr 参数，增加 sds 的长度，缩减空余空间，
 * 并将 \0 放到新字符串的尾端
 *
 * This function is used in order to fix the string length after the
 * user calls sdsMakeRoomFor(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * 这个函数是在调用 sdsMakeRoomFor() 对字符串进行扩展，
 * 然后用户在字符串尾部写入了某些内容之后，
 * 用来正确更新 free 和 len 属性的。
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * 如果 incr 参数为负数，那么对字符串进行右截断操作。
 *
 * Usage example:
 *
 * Using sdsIncrLen() and sdsMakeRoomFor() it is possible to mount the
 * following schema, to cat bytes coming from the kernel to the end of an
 * sds string without copying into an intermediate buffer:
 *
 * 以下是 sdsIncrLen 的用例：
 *
 * oldlen = sdslen(s);
 * s = sdsMakeRoomFor(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * sdsIncrLen(s, nread);
 *
 * 复杂度
 *  T = O(1)
 */
void sdsIncrLen(sds s, int incr) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    
    // 确保 sds 空间足够(assert 最好禁用 用if判断即可)
    if(sh->free < incr) {
        printf("incr is bigger than free in sds\n");
        exit(1);
    }
    
    // 更新len 跟free
    sh->len += incr;
    sh->free -= incr;
    
    // 最后放置新的结尾符号
    s[sh->len] = '\0';
}
/* Grow the sds to have the specified length. Bytes that were not part of
 * the original length of the sds will be set to zero.
 *
 * if the specified length is smaller than the current length, no operation
 * is performed. */
/*
 * 将 sds 扩充至指定长度，未使用的空间以 0 字节填充。
 *
 * 返回值
 *  sds ：扩充成功返回新 sds ，失败返回 NULL
 *
 * 复杂度：
 *  T = O(N)
 */
sds sdsgrowzero(sds s, size_t len) {
    struct sdshdr *sh = (void *)(s-(sizeof(struct sdshdr)));
    int current = sh->len;
    int total;
    if(len < current) {
        printf("扩充的长度比原先的小\n");
        exit(1);
    }
    //调用sdsMakeRoomFor来重新拓展s
    s = sdsMakeRoomFor(s, len-current);
    
    if(s == NULL)
        return NULL;
    sh = (void *)(s-sizeof(struct sdshdr));
    //void *memset(void *s, int ch, size_t n);memset是对一段内存快中s后面的n长度都填充ch
    memset(s+current, 0, len-current+1);
    total = sh->len+sh->free;
    sh->len = (int)len;
    sh->free = total-sh->len;
    return s;
}
