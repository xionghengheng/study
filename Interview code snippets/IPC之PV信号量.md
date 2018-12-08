### IPC之PV信号量

#### 消息队列定义和优势

信号量类似于计数器，是一个特殊的变量，值可以改变，但只能取正整数值，并且对它的加1和减1操作是原子操作。如果信号量值为0，那么再进行减1操作时会阻塞。信号量的初始值，代表资源的数量。 

作用：控制多个进程对临界资源的访问，使程序在同一个时刻，只有一个进程访问临界资源（进行进程间同步控制），原理就是控制程序的执行速度，起到一个协调公共资源的作用。 

```c
int semget(key_t key, int num_sems：, int sem_flags); //第一次使用时创建信号量，以后使用时获取信号量
int semctl(int sem_id, int sem_num, int command, ...);//对信号量值进行修改
int semop(int sem_id, struct sembuf *sem_ops,size_t num_sem_ops);
```



#### 其他关键定义

* 临界资源：同一时刻，只允许一个或有限个进程或线程访问的资源。 

  *   多个人同时用一个笔签字，此时只能有一个人用笔写字，其他人只有等他写完才可以使用这支笔。 
  *   若商场试衣间可以有3个试衣间，可以同时供3个人使用，其他人必须等到其中的试衣间没人才能使用。 

  *   但是像走廊，不是临界资源，可以同时由多人同时通行。 

* 临界区：访问临界资源的代码段。 

* 原子操作：不可被分割或中断的操作，操作一旦开始执行，就比执行结束，中途不能被任何原因打断。



#### 代码实现举例

模拟对打印机的使用，同一时刻只能一个人打印，printf代表打印，打印A开始，打印第二个A结束打印，打印A时不能打印B，可以出现4个A或4个B的情况，不能出现交错打印的情况。

思路：A先访问打印机时，B中的p操作阻塞住，当A执行完，进行v操作，B执行p操作，此时A中的p操作阻塞，当B执行完，执行v操作，A中的p操作就不会阻塞。 

输出结果：A使用完打印机后，会通知B，然后B再使用，借助于PV信号量，使得多个进程能合理按需的使用临界资源。

信号量创建的共有代码

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <unistd.h>
static int semid = 0;

//获取信号量
void sem_init()
{
    semid = semget((key_t)1234,1,IPC_CREAT | IPC_EXCL| 0600);//第一次使用时创建信号量，以后使用时获取信号量
    if(semid == -1)
    {
        semid = semget((key_t)1234,1,IPC_CREAT | 0600);
        if(semid == -1)
        {
            printf("semget error");
        }
    }
    else
    {
        union semun a;//a传值
        a.val = 1;
        if(semctl(semid,0,SETVAL,a)==-1)//对信号量值进行修改（0代表信号量下标）
        {
            perror("semctl init error");
        }
    }
}

//-1操作
void sem_p()
{
    struct sembuf buf;
    buf.sem_num = 0;//信号量下标
    buf.sem_op = -1;//p操作
    buf.sem_flg = SEM_UNDO;
    if(semop(semid,&buf,1)==-1)
    {
        perror("p error");
    }
}

//+1操作
void sem_v()
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;//设置在进程出现错误时信号量值自动恢复，防止一个进程占着信号量
    if(semop(semid,&buf,1)==-1)//1表示操作数，sembuf的数量
    {
        perror("v error");
    }
}

//删除操作
void sem_destroy()
{
    if(semctl(semid,0,IPC_RMID)==-1)//0代表信号量集
    {
        perror("semctl destroy error");
    }
}
```



打印机A

```c
int main()
{
    sem_init();
    int i = 0;
    for(;i<10;++i)
    {
        sem_p();
        printf("A");
        fflush(stdout);

        sleep(3);
        
        printf("A\r\n");
        fflush(stdout);
        
        sem_v();
        int n = rand()%3;
        sleep(n);
    }
}
```



打印机B

```c
int main()
{
    sem_init();
    int i = 0;
    for(;i<10;++i)
    {
        sem_p();
        printf("B");
        fflush(stdout);
        
        sleep(3);
        
        printf("B\r\n");
        fflush(stdout);
        
        sem_v();
        int n = rand()%3;//产生随机睡眠时间
        sleep(n);
    }
    sleep(10);
    sem_destroy();
}
```
