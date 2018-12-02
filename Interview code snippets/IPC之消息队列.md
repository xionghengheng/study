### IPC之消息队列

#### 消息队列定义和优势

消息队列可以认为是内核为不同进程间维护了一个消息队列的链表，由队列标识符标识。例如A进程创建了一个队列，B进程往同一个队列发消息的时候，A进程就能通过接收函数接收对应类型的消息

消息队列克服了信号传递信息少、管道只能承载无格式字节流以及缓冲区大小受限等缺点。


关键接口：创建消息队列、添加消息到消息队列、从消息队列取消息、删除消息队列

```c
int msgget(key_t key, int msgflg); //该函数用来创建和访问一个消息队列
```

与其他的IPC机制一样，程序必须提供一个键key来命名某个特定的消息队列，这样不同进程程序使用同一个key即可找到对应的消息队列。

msgflg是一个权限标志，表示消息队列的访问权限，它与文件的访问权限一样。msgflg可以与IPC_CREAT做或操作，表示当key所命名的消息队列不存在时创建一个消息队列，如果key所命名的消息队列存在时，IPC_CREAT标志会被忽略，而只返回一个标识符。

它返回一个以key命名的消息队列的标识符msgid（非零整数），失败时返回-1.



```c
int msgsend(int msgid, const void *msg_ptr, size_t msg_sz, int msgflg); //添加到消息队列中
```

msgid：是创建消息队列时返回的队列标识符

msg_ptr：指向准备发送的消息结构体的指针，该结构体有特殊要求，结构体第一个变量要求为消息类型

msg_sz：不包括类型在内的消息体的大小

msgflg：用于控制当前消息队列满或队列消息到达系统范围的限制时将要发生的事情



```c
int msgrcv(int msgid, void *msg_ptr, size_t msg_st, long int msgtype, int msgflg);//从消息队列获取消息
```

msgtype：可以实现一种简单的接收优先级。如果msgtype为0，就获取队列中的第一个消息。如果它的值大于零，将获取具有相同消息类型的第一个信息。如果它小于零，就获取类型等于或小于msgtype的绝对值的第一个消息。**（可以做消息过滤）**



```c
int msgctl(int msgid, int command, struct msgid_ds *buf);//控制消息队列
```

command：是将要采取的动作，它可以取3个值

- IPC_STAT：把msgid_ds结构中的数据设置为消息队列的当前关联值，即用消息队列的当前关联值覆盖msgid_ds的值。
- IPC_SET：如果进程有足够的权限，就把消息列队的当前关联值设置为msgid_ds结构中给出的值
- IPC_RMID：删除消息队列

#### 实现

以两个程序举例来看消息队列的实现，一个是消息发送进程程序，一个是消息接受者进程程序

消息发送者

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>

#define MAX_TEXT 512
struct msg_st
{
    long int msg_type;//指针msg_ptr所指向的消息结构一定要是以一个长整型成员变量开始的结构体，接收函数将用这个成员来确定消息的类型
    char text[MAX_TEXT];
};

int main()
{
    int running = 1;
    struct msg_st data;
    char buffer[BUFSIZ];
    int msgid = -1;
    
    //建立消息队列
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
    if(msgid == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    //向消息队列中写消息，直到写入end
    while(running)
    {
        //输入数据
        printf("Enter some text: ");
        fgets(buffer, BUFSIZ, stdin);
        data.msg_type = 3;    //注意取值
        strcpy(data.text, buffer);
        //向队列发送数据
        if(msgsnd(msgid, (void*)&data, MAX_TEXT, 0) == -1)
        {
            fprintf(stderr, "msgsnd failed\n");
            exit(EXIT_FAILURE);
        }
        //输入end结束输入
        if(strncmp(buffer, "end", 3) == 0)
            running = 0;
        sleep(1);
    }
    exit(EXIT_SUCCESS);
}

```



消息接收者

```c
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

struct msg_st
{
    long int msg_type;
    char text[BUFSIZ];
};

int main()
{
    int running = 1;
    int msgid = -1;
    struct msg_st data;
    long int msgtype = 3; //如果为0表示不care类型，直接取消息队列第一个元素，大于0则表明要从消息队列取和msgtype对应的消息体
    
    //建立消息队列
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        fprintf(stderr, "msgget failed with error: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    //从队列中获取消息，直到遇到end消息为止
    while(running)
    {
        if (msgrcv(msgid, (void*)&data, BUFSIZ, msgtype, 0) == -1)
        {
            fprintf(stderr, "msgrcv failed with errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
        printf("You wrote: %s\n",data.text);
        //遇到end结束
        if (strncmp(data.text, "end", 3) == 0)
            running = 0;
    }
    //删除消息队列
    if (msgctl(msgid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
```
