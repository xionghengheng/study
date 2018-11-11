参考自：https://github.com/shineyr/Socket和https://github.com/Jiavan/httpclient-cli/tree/master/src

### 简易TCP客户端/服务端通信程序

两段程序说明了：TCP链接的建立过程以及数据读写的流程，整个TCP三次握手建立链接的过程如图所示。

![tcp](./tcp.png)



客户端的系统调用流程为（比较简单）：

* 创建socket套接字和服务器地址结构 socket()
* 发起链接 connect()
* 收发数据 read()/write()

服务端的系统调用流程为：

- 创建socket套接字listenfd socket()
- 绑定套接字listenfd和服务端地址结构 bind()
- 将套接字listenfd变为监听套接字，修改套接字属性 listen()
- 阻塞等待客户端链接 accept()
- 收发数据 read()/write()

```c++
/*
 * client.c为客户端代码(代码完全来自shineyr的github，居然是个妹子~)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

const int MAX_LINE = 2048;
const int PORT = 6000;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;

/*readline函数实现*/
ssize_t readline(int fd, char *vptr, size_t maxlen)
{
    ssize_t n, rc;
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


int main()
{
    /*声明套接字和链接服务器地址*/
    const char* ip = "127.0.0.1";
    int sockfd;
    struct sockaddr_in servaddr;
    
    
    /*(1) 创建套接字*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)//AF_INET指定IPV4，SOCK_STREAM指定TCP协议
    {
        perror("socket error");
        exit(1);
    }
    
    /*(2) 设置链接服务器地址结构*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0)//inet_pton把点分十进制字符串转换为二进制整型
    {
        printf("inet_pton error for %s\n", ip);
        exit(1);
    }
    
    /*(3) 发送链接服务器请求*/
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect error");
        exit(1);
    }
    
    /*(4) 消息处理*/
    char sendline[MAX_LINE], recvline[MAX_LINE];
    while(fgets(sendline, MAX_LINE, stdin) != NULL)
    {
        write(sockfd, sendline, strlen(sendline));
        
        if(readline(sockfd, recvline, MAX_LINE) == 0)
        {
            perror("server terminated prematurely");
            exit(1);
        }
        
        if(fputs(recvline , stdout) == EOF)
        {
            perror("fputs error");
            exit(1);
        }
    }
    
    /*(5) 关闭套接字*/
    close(sockfd);
}
```



```c++
/*
 * server.c为服务器端代码
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

const int MAX_LINE = 2048;
const int PORT = 6000;
const int BACKLOG = 10;
const int LISTENQ = 6666;
const int MAX_CONNECT = 20;

int main(int argc , char **argv)
{
    /*声明服务器地址和客户链接地址*/
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    
    /*声明服务器监听套接字和客户端链接套接字*/
    int listenfd, connfd;
    pid_t childpid;
    
    /*声明缓冲区*/
    char buf[MAX_LINE];
    
    /*(1) 初始化监听套接字listenfd*/
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }
    
    /*(2) 设置服务器sockaddr_in结构*/
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //表明可接受任意IP地址
    servaddr.sin_port = htons(PORT);
    
    /*(3) 绑定监听套接字和端口*/
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind error");
        exit(1);
    }
    
    /*(4) 监听客户请求*/
    if (listen(listenfd, LISTENQ) < 0)
    {
        perror("listen error");
        exit(1);
    }
    
    /*(5) 接受客户请求*/
    for( ; ; )
    {
        clilen = sizeof(cliaddr);
        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0 )
        {
            perror("accept error");
            exit(1);
        }
        printf("listenfd=%d connfd=%d\r\n", listenfd, connfd);
        //新建子进程单独处理链接
        if((childpid = fork()) == 0)
        {
            close(listenfd);
            //str_echo
            ssize_t n;
            char buff[MAX_LINE];
            while((n = read(connfd, buff, MAX_LINE)) > 0)
            {
                write(connfd, buff, n);
            }
            exit(0);
        }
        close(connfd);
    }
    
    /*(6) 关闭监听套接字*/
    close(listenfd);
}
```

