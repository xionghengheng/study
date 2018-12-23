### IPC之共享内存

#### 共享内存定义

为了在多个进程间交换信息，内核专门留出了一块内存区，可以由需要访问的进程将其映射到自己的私有地址空间。采用共享内存通信的一个显而易见的好处是效率高，因为进程可以直接读写内存，而不需要任何数据的拷贝。对于像管道和消息队列等通信方式，则需要在内核和用户空间进行四次的数据拷贝，而共享内存则只拷贝两次数据：一次从输入文件到共享内存区，另一次从共享内存区到输出文件，从而大大提高效率。

由于多个进程共享一段内存，因此需要依靠某种同步机制（如信号量）来达到进程间的同步及互斥。

实现方式通常有三种，具体如下。

#### mmap内存映射

mmap()系统调用使得进程之间通过映射同一个普通文件实现共享内存。普通文件被映射到进程地址空间后，进程可以像访问普通内存一样对文件进行访问，不必再调用read，write等操作。

函数说明：

```c++
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
void *mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset);
```

addr：指定文件应被映射到进程空间的起始地址，一般被指定一个空指针，此时选择起始地址的任务留给内核来完成。函数的返回值为最后文件映射到进程空间的地址，进程可直接操作起始地址为该值的有效地址。

length：映射到调用进程地址空间的字节数，它从被映射文件开头offset个字节开始算起。

prot：指定共享内存的访问权限。可取如下几个值的或：PROT_READ（可读），PROT_WRITE（可写），PROT_EXEC（可执行），PROT_NONE（不可访问）。

fd：为即将映射到进程空间的文件描述字，一般由open()返回，同时，fd可以指定为-1，此时须指定flags参数中的MAP_ANON，表明进行的是匿名映射（不涉及具体的文件名，避免了文件的创建及打开，很显然只能用于具有亲缘关系的进程间通信）

offset：一般设为0，表示从文件头开始映射。

```c++
int munmap(void *addr, size_t length);//该调用在进程地址空间中解除一个映射关系，addr是调用mmap()时返回的地址，length是映射区的大小。当映射关系解除后，对原来映射地址的访问将导致段错误发生。
int msync(void *addr, size_t length, int flags);//进程在映射空间的对共享内容的改变并不直接写回到磁盘文件中，往往在调用munmap后才执行该操作。可以通过调用msync实现磁盘上文件内容与共享内存区的内容一致。
int ftruncate(int fd, off_t length);//调整fd所指的文件的大小到length
int fstat(int fd, struct stat *buf); //获取fd所指的文件的详细信息
```



写共享内存

```c++
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct
{
    char name[32];
    int age;
} people;

main(int argc, char** argv)
{
    people* p_map;
    char temp = 'a';
    
    int fd = open(argv[1], O_CREAT|O_RDWR|O_TRUNC, 00777);
    if (-1 == fd)
    {
        printf("open file error = %s\n", strerror(errno));
        return -1;
    }
    ftruncate(fd, sizeof(people)*10);
    p_map = (people*)mmap(NULL, sizeof(people)*10, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == p_map)
    {
        printf("mmap file error = %s\n", strerror(errno));
        return -1;
    }
    
    for(int i = 0; i < 10; i++)
    {
        memcpy( ( *(p_map+i) ).name, &temp, 1);
        ( *(p_map+i) ).name[1] = 0;
        ( *(p_map+i) ).age = 20+i;
        temp += 1;
    }
    printf("initialize over\n");
        
    close(fd);
    munmap(p_map, sizeof(people)*10);
    printf("umap ok \n");
    return 0;
}
```



读共享内存

```c++
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

typedef struct
{
    char name[32];
    int age;
} people;

main(int argc, char** argv)
{
    people* p_map;
    struct stat filestat;
    
    int fd = open(argv[1], O_CREAT|O_RDWR, 00777);
    if (-1 == fd)
    {
        printf("open file error = %s\n", strerror(errno));
        return -1;
    }
    fstat(fd, &filestat);
    p_map = (people*)mmap(NULL, filestat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (MAP_FAILED == p_map)
    {
        printf("mmap file error = %s\n", strerror(errno));
        return -1;
    }
    
    for(int i = 0; i < 10; i++)
    {
        printf("name = %s, age = %d\n",(*(p_map+i)).name, (*(p_map+i)).age);
    }
    
    close(fd);
    munmap(p_map, sizeof(people)*10);
    printf("umap ok \n");
    return 0;
}
```



#### Posix共享内存区

POSIX共享内存使用方法有以下两个步骤：

- 通过shm_open创建或打开一个POSIX共享内存对象

- 调用mmap将它映射到当前进程的地址空间

和通过内存映射文件进行通信的使用上差别在于，mmap描述符参数获取方式不一样：是通过shm_open获取共享文件的描述符

接口说明：

```c
int shm_open(const char *name, int oflag, mode_t mode);// 打开一个共享内存的文件句柄。注意这里的名字具有形式/somename，即必须以/为开头，因为POSIX共享内存对应的文件是位于/dev/shm这个特殊文件系统内。
int shm_unlink(const char *name);// 删除一个共享内存的名字，但只有所有程序都关闭，才会真的删除。
```



#### SystemV

接口说明：

```c
int shmget(key_t key, size_t size, int shmflg);// 获取共享内存区域
void *shmat(int shmid, const void *shmaddr, int shmflg);// 连接共享内存区域
int shmdt(const void *shmaddr);// 断开共享内存区域
int shmctl(int shmid, int cmd, struct shmid_ds *buf);// 对共享内存区域进行控制
key_t ftok(const char *pathname, int proj_id);// 将path和proj_id转换成System V IPC key
```







#### 总结

共享内存是最快的IPC形式，在开发中，我们一定要充分利用好共享内存的特性，取得事半功倍的效果。

| 类型                  | 原理                                        | 易失性                       |
| --------------------- | ------------------------------------------- | ---------------------------- |
| mmap                  | 利用文件(open)映射共享内存区域              | 会保存在磁盘上，不会丢失     |
| Posix shared memory   | 利用/dev/shm文件系统(mmap)映射共享内存区域  | 随内核持续，内核自举后会丢失 |
| SystemV shared memory | 利用/dev/shm文件系统(shmat)映射共享内存区域 | 随内核持续，内核自举后会丢失 |

 

 

文章来源：http://www.cnblogs.com/linuxbug/p/4882776.html
