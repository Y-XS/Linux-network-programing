# 参考资料

+ TCP/IP 网络编程

# 常用协议

HTTP、FTP、TCP/IP、ARP、

## TCP

+ 三次握手
+ 四次挥手
+ 半关闭
+ 滑动窗口

## IP



# 常用工具

## netcat

```sh
nc 127.1 9527
```



# 辅助函数

## atoi

```c
//ascii to integer
int atoi(const char *nptr);
//template
int n = atoi("123");
```



# socket

一个文件描述符指向一个套接字（该套接字内部由内核借助两个缓冲区实现）



## 预备知识

### 网络字节序

+ 小端字节序（主机）：高位字节存储在高地址处，低位字节存储在低地址处
+ 大端字节序（网络）：一个整数的高位字节（23-31 bit）存储在内存的低地址处，低位字节（0-7 bit）存储在内存的高地址处

```c
/* 
	h：host 主机
	n：network 网络
	l：32位长整数 4字节
	s：16位短整数 2字节
*/
//本地-->网络（IP）
uint32_t htonl(uint32_t hostlong);
//本地-->网络（port）
uint16_t htons(uint16_t hostshort);
//网络-->本地（IP）
uint32_t ntohl(uint32_t netlong);
//网络-->本地（port）
uint16_t ntohs(uint16_t netshort);
```

### IP地址转换函数

```c
//IP地址转换函数是为了简化以下步骤
//192.168.1.11 -----atoi------>    int    -----htonl----->网络字节序
//	string          		   主机字节序
```

```c
/*
description：主机（string IP）-->网络
args:
	af：AF_INET、AF_INET6
	src：IP地址（点分十进制）
	dst：转换后的网络字节序的IP地址
return:
	success：1
	error：-1
*/
int inet_pton(int af,const char *src,void *dst);
/*
args：
	af：AF_INET、AF_INET6
	src：IP地址（网络字节序）
	dst：本地字节序（点分十进制）
	size：dst的大小
return：
	success：dst
	fail：NULL
*/
const char* inet_ntop(int af,const void* src,char *dst,socklen_t size);
```

### sockaddr

> man 7 ip

```c
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(9527);
addr.sin_addr.s_addr = htonl(INADDR_ANY);
```

## 连接模型

![](imgs/socket.png)

| server：    |                        |
| ----------- | ---------------------- |
| 1. socket() | 创建socket             |
| 2. bind()   | 绑定服务器地址结构     |
| 3. listen() | 设置监听上限           |
| 4. accept() | 阻塞监听客户端连接     |
| 5. read()   | 读socket获取客户端数据 |
| 6. func()   | 业务函数               |
| 7. write()  |                        |
| 8. close()  |                        |

| client：     |                  |
| ------------ | ---------------- |
| 1. socket()  | 创建socket       |
| 2. connect() | 与服务器建立连接 |
| 3. write()   | 写数据到socket   |
| 4. read()    | 读数据           |
| 5. close()   | 关闭socket       |



## socket

```c
/*
args:
	domain：AF_INET、AF_INET6、AF_UNIX
	type：SOCK_STREAM、SOCK_DGRAM
	protocol：0
*/
int socket(int domain,int type,int protocol);
```

## bind

绑定服务器地址结构

```c
/*
args:
	sockfd：socket函数返回值
	addr：(strruct sockaddr*)&addr
	addrlen：sizeof(addr) 地址结构的大小
*/
int bind(int sockfd,const struct sockaddr *addr,socklen_t addrlen);
```

## listen

设置同时与服务器建立连接的上限数（同时进行3次握手的客户端数量）

```c
/*
args:
	sockfd：socket函数返回值
	backlog：连接上限。最大值128
*/
int listen(int sockfd,int backlog);
```

## accept

阻塞等待客户端建立连接，成功的话，返回一个与客户端成功连接的socket文件描述符。

```c
/*
args:
	sockfd：socket函数返回值
	addr：传出与服务器建立连接的客户端的地址结构
	addrlen：传入addr，传出客户端addr实际大小
*/
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

## connect

使用socket与服务器建立连接

如果不使用bind绑定客户端地址结构，则系统采用“隐式绑定”

```c
int connect(int sockfd,const struct sockaddr *addr, socklen_t addrlen);
```

## template

```c
//server
int sfd,cfd;
int ret=0;
socklen_t caddrlen;
char buf[BUFSIZ];
//填写地址信息
struct sockaddr_in saddr,caddr;
saddr.sin_family = AF_INET;
saddr.sin_port = htons(8888);
saddr.sin_addr.s_addr = htonl(INADDR_ANY);
//创建socket
sfd = socket(AF_INET,SOCK_STREAM,0);
sys_err(sfd,"create socket error");
//绑定地址信息
ret = bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
sys_err(ret,"bind error");
//设置监听上限
ret = listen(sfd,5);
sys_err(ret,"listen error");
//阻塞监听客户端连接
caddrlen = sizeof(caddr);
cfd = accept(sfd,(struct sockaddr*)&caddr,&caddrlen);
sys_err(cfd,"listen error");
//打印客户端地址信息
cout<<"client ip: "
	<<inet_ntop(AF_INET,&caddr.sin_addr.s_addr,client_ip_buf,sizeof(client_ip_buf))
    <<" port: "
    <<ntohs(caddr.sin_port)
    <<endl;
while(1){
    //读取客户端数据
    int n = read(cfd,buf,sizeof(buf));
    //业务函数
    for(int i=0;i<n;i++){
        buf[i] = toupper(buf[i]);
    }
    //写出数据
    write(cfd,buf,n);
}    
//关闭socket
close(sfd);
close(cfd);
```

```c
//client
int cfd,ret;
int counter = 5;
const char *sip = "127.0.0.1";
char buf[BUFSIZ];
//填写服务器地址信息
struct sockaddr_in saddr;
saddr.sin_family = AF_INET;
saddr.sin_port = htons(SERV_PORT);
inet_pton(AF_INET,sip,&saddr.sin_addr.s_addr);
//创建socket
cfd = socket(AF_INET,SOCK_STREAM,0);
sys_err(cfd,"create socket error");
//连接服务器
ret = connect(cfd,(struct sockaddr *)&saddr,sizeof(saddr));
sys_err(ret,"connect error");
while(counter--){
    //业务函数
    write(cfd,"hello\n",6);
    ret = read(cfd,buf,sizeof(buf));
    write(STDOUT_FILENO,buf,ret);
    sleep(1);
}
//关闭socket
close(cfd);
return 0;
```



# 并发

## 多进程

## 多线程





