//
//  main.c
//  redis_resource
//
//  Created by pengge on 17/3/13.
//  Copyright © 2017年 pengge. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "sds.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    //根据len，创建字符串a
    sds a = sdsnewlen("dds",3);
    //自动创建字符串b
    sds b = sdsnew("sdfsdfsf");
    //为c在b的基础上申请1mb的空间
    sds c = sdsMakeRoomFor(b,1024*1024);
    //计算c的可用空间
    printf("c length=%zu\n",sdslen(c));
    printf("c available=%zu\n",sdsavail(c));
    //移除c的可以空间，创建d
    sds d = sdsRemoveFreeSpace(c);
    printf("d =%s\n",d);
    //查看d的可用空间
    printf("d length=%zu\n",sdsavail(d));
    //计算e分配的内存数＝（sizeof(struct sdshdr)+sds->len+len->free+1）
    size_t e = sdsAllocSize(d);
    printf("e lenth = %zu\n",e);
    
    //拓展c的长度(同时需要减少c的可用空间)
    sdsIncrLen(c,5);
    printf("c length=%zu\n",sdslen(c));
    printf("c available=%zu\n",sdsavail(c));
    
    printf("Hello, World!\n");
    return 0;
}
