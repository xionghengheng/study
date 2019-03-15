### IO多路复用之poll

接口介绍：

```c
#include <sys/epoll.h>
int epoll_create(int size);//创建epoll句柄
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);//epoll的事件注册
int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);//等待事件产生
```

#### 创建epoll句柄

创建一个epoll的句柄，size用来告诉内核这个监听的数目一共有多大。这个参数不同于select()中的第一个参数，给出最大监听的fd+1的值。需要注意的是，当创建好epoll句柄后，它就是会占用一个fd值，在linux下如果查看/proc/进程id/fd/，是能够看到这个fd的，所以在使用完epoll后，必须调用close()关闭，否则可能导致fd被耗尽。

#### epoll的事件注册
epoll的事件注册函数，它不同于select()是在监听事件时告诉内核要监听什么类型的事件，而是在这里先注册要监听的事件类型。第一个参数是epoll_create()的返回值，第二个参数表示动作，用三个宏来表示：

EPOLL_CTL_ADD：注册新的fd到epfd中；

EPOLL_CTL_MOD：修改已经注册的fd的监听事件；

EPOLL_CTL_DEL：从epfd中删除一个fd；

第三个参数是需要监听的fd，第四个参数是告诉内核需要监听什么事，struct epoll_event结构如下：

```c
struct epoll_event {
  __uint32_t events;  /* Epoll events */
  epoll_data_t data;  /* User data variable */
};
```

events可以是以下几个宏的集合：

EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；

EPOLLOUT：表示对应的文件描述符可以写；

EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；

EPOLLERR：表示对应的文件描述符发生错误；

EPOLLHUP：表示对应的文件描述符被挂断；

EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的

EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里

#### 等待事件产生

等待事件的产生，类似于select()调用。参数events用来从内核得到事件的集合，maxevents告之内核这个events有多大，这个maxevents的值不能大于创建epoll_create()时的size，参数timeout是超时时间（毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。该函数返回需要处理的事件数目，如返回0表示已超时。

epfd：为epoll_create创建之后的句柄
events：是一个返回值，返回值里存储了所有的读写事件
max_events：当前需要监听的最大的socket句柄数
time_out：超时等待时间。（毫秒，值为0则会立即返回，值为-1则会一直阻塞，值为任意正整数x，则表示等待x后返回）


#### epoll工作模式选择

epoll对文件描述符的操作有两种模式：LT（level trigger）和ET（edge trigger）。LT模式是默认模式，LT模式与ET模式的区别如下：

* LT模式：当epoll_wait检测到描述符事件发生并将此事件通知应用程序，应用程序可以不立即处理该事件。下次调用epoll_wait时，会再次响应应用程序并通知此事件。
* ET模式：当epoll_wait检测到描述符事件发生并将此事件通知应用程序，应用程序必须立即处理该事件。如果不处理，下次调用epoll_wait时，不会再次响应应用程序并通知此事件。

ET模式在很大程度上减少了epoll事件被重复触发的次数，因此效率要比LT模式高。epoll工作在ET模式的时候，必须使用非阻塞套接口，以避免由于一个文件句柄的阻塞读/阻塞写操作把处理多个文件描述符的任务饿死。



Epoll服务端代码：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


#ifndef CONNECT_SIZE
#define CONNECT_SIZE 256
#endif

#define PORT 7777
#define MAX_LINE 2048
#define LISTENQ 20

void setNonblocking(int sockfd)
{
    int opts;
    opts=fcntl(sockfd,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        return;
    }//if
    
    opts = opts|O_NONBLOCK;
    if(fcntl(sockfd,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return;
    }//if
}

int main(int argc , char **argv)
{
    int i, listenfd, connfd, sockfd, epfd, nfds;
    
    ssize_t n, ret;
    
    char buf[MAX_LINE];
    
    socklen_t clilen;
    
    struct sockaddr_in servaddr , cliaddr;
    
    /*声明epoll_event结构体变量，ev用于注册事件，数组用于回传要处理的事件*/
    struct epoll_event ev, events[20];
    
    /*(1) 得到监听描述符*/
    listenfd = socket(AF_INET , SOCK_STREAM , 0);
    setNonblocking(listenfd);
    
    /*生成用于处理accept的epoll专用文件描述符*/
    epfd = epoll_create(CONNECT_SIZE);
    /*设置监听描述符*/
    ev.data.fd = listenfd;
    /*设置处理事件类型*/
    ev.events = EPOLLIN | EPOLLET;
    /*注册事件*/
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    
    /*(2) 绑定套接字*/
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
    
    bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr));
    
    /*(3) 监听*/
    listen(listenfd , LISTENQ);
    
    /*(4) 进入服务器接收请求死循环*/
    while(1)
    {
        /*等待事件发生*/
        nfds = epoll_wait(epfd , events , CONNECT_SIZE , -1);
        if(nfds <= 0)
            continue;
        
        printf("nfds = %d\n" , nfds);
        /*处理发生的事件*/
        for(i=0 ; i<nfds ; ++i)
        {
            /*检测到用户链接*/
            if(events[i].data.fd == listenfd)
            {
                /*接收客户端的请求*/
                clilen = sizeof(cliaddr);
                
                if((connfd = accept(listenfd , (struct sockaddr *)&cliaddr , &clilen)) < 0)
                {
                    perror("accept error.\n");
                    exit(1);
                }//if
                
                printf("accpet a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr) , cliaddr.sin_port);
                
                /*设置为非阻塞*/
                setNonblocking(connfd);
                ev.data.fd = connfd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd , EPOLL_CTL_ADD , connfd , &ev);
            }//if
            /*如果是已链接用户，并且收到数据，进行读入*/
            else if(events[i].events & EPOLLIN){
                
                if((sockfd = events[i].data.fd) < 0)
                    continue;
                bzero(buf , MAX_LINE);
                printf("reading the socket~~~\n");
                if((n = read(sockfd , buf , MAX_LINE)) <= 0)
                {
                    close(sockfd);
                    events[i].data.fd = -1;
                }//if
                else{
                    buf[n] = '\0';
                    printf("clint[%d] send message: %s\n", i , buf);
                    
                    /*设置用于注册写操作文件描述符和事件*/
                    ev.data.fd = sockfd;
                    ev.events = EPOLLOUT| EPOLLET;
                    epoll_ctl(epfd , EPOLL_CTL_MOD , sockfd , &ev);
                }//else
            }//else
            else if(events[i].events & EPOLLOUT)
            {
                if((sockfd = events[i].data.fd) < 0)
                    continue;
                if((ret = write(sockfd , buf , n)) != n)
                {
                    printf("error writing to the sockfd!\n");
                    break;
                }//if
                /*设置用于读的文件描述符和事件*/
                ev.data.fd = sockfd;
                ev.events = EPOLLIN | EPOLLET;
                /*修改*/
                epoll_ctl(epfd , EPOLL_CTL_MOD , sockfd , &ev);
            }//else
        }//for
    }//while
    free(events);
    close(epfd);
    exit(0);
}
```



Epoll客户端代码：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#define PORT 7777
#define MAX_LINE 2048

int max(int a , int b)
{
    return a > b ? a : b;
}

/*readline函数实现*/
ssize_t readline(int fd, char *vptr, size_t maxlen)
{
    ssize_t    n, rc;
    char    c, *ptr;
    
    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = read(fd, &c,1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;    /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return(n - 1);    /* EOF, n - 1 bytes were read */
        } else
            return(-1);        /* error, errno set by read() */
    }
    
    *ptr = 0;    /* null terminate like fgets() */
    return(n);
}

/*普通客户端消息处理函数*/
void str_cli(int sockfd)
{
    /*发送和接收缓冲区*/
    char sendline[MAX_LINE] , recvline[MAX_LINE];
    while(fgets(sendline , MAX_LINE , stdin) != NULL)
    {
        write(sockfd , sendline , strlen(sendline));
        
        bzero(recvline , MAX_LINE);
        if(readline(sockfd , recvline , MAX_LINE) == 0)
        {
            perror("server terminated prematurely");
            exit(1);
        }//if
        
        if(fputs(recvline , stdout) == EOF)
        {
            perror("fputs error");
            exit(1);
        }//if
        
        bzero(sendline , MAX_LINE);
    }//while
}

int main(int argc , char **argv)
{
    /*声明套接字和链接服务器地址*/
    int sockfd;
    struct sockaddr_in servaddr;
    
    /*判断是否为合法输入*/
    if(argc != 2)
    {
        perror("usage:tcpcli <IPaddress>");
        exit(1);
    }//if
    
    /*(1) 创建套接字*/
    if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }//if
    
    /*(2) 设置链接服务器地址结构*/
    bzero(&servaddr , sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET , argv[1] , &servaddr.sin_addr) < 0)
    {
        printf("inet_pton error for %s\n",argv[1]);
        exit(1);
    }//if
    
    /*(3) 发送链接服务器请求*/
    if(connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }//if
    
    /*调用消息处理函数*/
    str_cli(sockfd);
    exit(0);
}

```
