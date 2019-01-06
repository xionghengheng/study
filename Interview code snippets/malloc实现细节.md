# malloc朴素版实现

以下代码实现了一个简单版本的malloc和free，但是基本思路和glibc实现的一致，使用一个双链表，维护每一个通过系统调用sbrk申请的堆内存的block，有分裂策略、合并策略等。

malloc具体实现流程如下：

* 查找双链表中合适的block，譬如使用first fit，找到首个符合大小的block
* 如果能找到，则直接使用，如果找到的block过大，则需要分裂为两个节点，避免内存浪费
* 如果找不到，则新建一个block，移动break指针申请新的内存块(通过sbrk系统调用移动break指针)

free实现流程如下：

* 首先检查参数地址的合法性，如果不合法则不做任何事；
* 如果合法，则将此block的free标为1，并且在可以的情况下与后面的block进行合并。
* 如果当前是最后一个block，则回退break指针释放进程内存，如果当前block是最后一个block，则回退break指针并设置双链表头节点first_block为NULL

```c++
/* Per thread arena example. */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>

/* 由于存在虚拟的data字段，sizeof不能正确计算meta长度，这里手工设置 */
#define BLOCK_SIZE 40

struct s_block {
    size_t size;   /* 数据区大小 8字节 */
    s_block* prev; /* 指向上个块的指针 */
    s_block* next; /* 指向下个块的指针 */
    int free;      /* 是否是空闲块 */
    int padding;   /* 填充4字节，保证meta块长度为8的倍数 */
    void *ptr;     /* Magic pointer，指向data */
    char data[1];  /* 这是一个虚拟字段，表示数据块的第一个字节，长度不应计入meta */
};

s_block* first_block = NULL; /* 双链表头节点 */

size_t align8(size_t s)
{
    if((s & 0x7) == 0)
        return s;
    return ((s >> 3) + 1) << 3;
}

/* 现在考虑如何在block链中查找合适的block。一般来说有两种查找算法：
 First fit：从头开始，使用第一个数据区大小大于要求size的块所谓此次分配的块
 Best fit：从头开始，遍历所有块，使用数据区大小大于size且差值最小的块作为此次分配的块
 两种方法各有千秋，best fit具有较高的内存使用率（payload较高），而first fit具有更好的运行效率。这里我们采用first fit算法。
 */
s_block* find_block(s_block** last, size_t size)
{
    s_block* b = first_block;
    while(b && !(b->free && b->size >= size))
    {
        *last = b;
        b = b->next;
    }
    return b;
}

/*如果现有block都不能满足size的要求，则需要在链表最后开辟一个新的block。这里关键是如何只使用sbrk创建一个struct*/
s_block* extend_heap(s_block* last, size_t s)
{
    s_block* b = (s_block*)sbrk(0);
    if (sbrk(BLOCK_SIZE + s) == (void *)-1)
        return NULL;
    b->size = s;
    b->next = NULL;
    if (last)
    {
        last->next = b;
        b->prev = last;
    }
    b->free = 0;
    b->ptr = b->data;
    return b;
}

/*First fit有一个比较致命的缺点，就是可能会让很小的size占据很大的一块block，
 此时，为了提高payload，应该在剩余数据区足够大的情况下，将其分裂为一个新的block
 */
void split_block(s_block* b, size_t s)
{
    s_block* newblock;
    newblock = (s_block*)(b->data + s);
    newblock->size = b->size - s - BLOCK_SIZE;
    newblock->next = b->next;
    newblock->next->prev = newblock;
    newblock->prev = b;
    newblock->free = 1;
    b->size = s;
    b->next = newblock;
}


void* malloc(size_t size)
{
    s_block* b = nullptr;
    s_block* last = nullptr;
    size_t s;
    /* 对齐地址 */
    s = align8(size);
    if (first_block)
    {
        /* 查找合适的block */
        last = first_block;
        b = find_block(&last, s);
        if (b)
        {
            /* 剩余空间至少有BLOCK_SIZE + 8才执行分裂操作 */
            if ((b->size - s) >= ( BLOCK_SIZE + 8))
                split_block(b, s);
            b->free = 0;
        }
        else
        {
            /* 没有合适的block，开辟一个新的 */
            b = extend_heap(last, s);
            if(!b)
                return NULL;
        }
    }
    else
    {
        b = extend_heap(NULL, s);
        if (!b)
            return NULL;
        first_block = b;
    }
    return b->data;
}


s_block* fusion(s_block* b)
{
    if (b->next && b->next->free)
    {
        b->size += BLOCK_SIZE + b->next->size;
        b->next = b->next->next;
        if (b->next)
            b->next->prev = b;
    }
    return b;
}

s_block* get_block(void *p)
{
    char* tmp = (char*)p;
    return (s_block*)(p = (tmp -= BLOCK_SIZE));
}


/*保证传入的地址有效性：
 1.地址应该在之前malloc所分配的区域内，即在first_block和当前break指针范围内
 2.这个地址确实是之前通过我们自己的malloc分配的
 */
int valid_addr(void *p)
{
    if (first_block)
    {
        if (p > first_block && p < sbrk(0))
        {
            return (p == (get_block(p)->ptr));
        }
    }
    return 0;
}

/*
 首先检查参数地址的合法性，如果不合法则不做任何事；否则，将此block的free标为1，并且在可以的情况下与后面的block进行合并。
 如果当前是最后一个block，则回退break指针释放进程内存，如果当前block是最后一个block，则回退break指针并设置first_block为NULL
 */
void free(void *p)
{
    s_block* b;
    if (valid_addr(p))
    {
        b = get_block(p);
        b->free = 1;
        if (b->prev && b->prev->free)
            b = fusion(b->prev);
        if (b->next)
        {
            fusion(b);
        }
        else
        {
            if (b->prev)
                b->prev->prev = NULL;
            else
                first_block = NULL;
            brk(b);
        }
    }
}


int main()
{
    char* p = (char*)malloc(100);
    memset(p, 0, 100);
    *p = 'f';
    
    char* p1 = (char*)malloc(20);
    memset(p1, 0, 20);
    *p1 = 'u';
    free(p);
    
    char* p3 = (char*)malloc(50);
    memset(p3, 0, 50);
    *p1 = 'c';
    free(p3);
    
    printf("%s\n", p);
}
```



以上是一个较为简陋，但是初步可用的malloc实现。还有很多遗留的可能优化点，例如：

- 同时兼容32位和64位系统
- 在分配较大快内存时，考虑使用mmap而非sbrk，这通常更高效
- 可以考虑维护多个链表而非单个，每个链表中的block大小均为一个范围内，例如8字节链表、16字节链表、24-32字节链表等等。此时可以根据size到对应链表中做分配，可以有效减少碎片，并提高查询block的速度
- 可以考虑链表中只存放free的block，而不存放已分配的block，可以减少查找block的次数，提高效率

# 参考链接

http://blog.codinglabs.org/articles/a-malloc-tutorial.html