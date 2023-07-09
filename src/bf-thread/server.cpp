#include<iostream>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
using namespace std;

#define SERV_PORT 9999
//客户端信息结构体
struct client_info{
    int connfd;
    struct sockaddr_in addr;
};
//子线程回调函数
void *callback(void *arg){
    struct client_info *info = (struct client_info *)arg;
    char buf[BUFSIZ];

    //打印客户端地址信息
    cout<<info->connfd
        <<" client ip: "
        <<inet_ntop(AF_INET,&info->addr.sin_addr.s_addr,buf,sizeof(buf))
        <<" port: "
        <<ntohs(info->addr.sin_port)
        <<endl;
    //业务逻辑
    while (1)
    {
        //读取客户端数据
        int n = read(info->connfd,buf,sizeof(buf));
        if(n==0)
            break;
        for(int i=0;i<n;i++){
            buf[i] = toupper(buf[i]);
        }
        //写出数据
        write(info->connfd,buf,n);
        write(STDOUT_FILENO,buf,n);
    }
    pthread_exit(info);
}

int main(){
    int sfd,cfd;
    int ret=0,i=0;
    socklen_t caddrlen;
    pthread_t tid;
    struct client_info infos[156];

    //填写地址信息
    struct sockaddr_in saddr,caddr;
    bzero(&saddr,sizeof(saddr));    //地址结构清零
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(SERV_PORT);
    //创建socket
    sfd = socket(AF_INET,SOCK_STREAM,0);
    //绑定地址信息
    ret = bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
    //设置监听上限
    ret = listen(sfd,5);

    caddrlen = sizeof(caddr);
    while(1){
        //阻塞监听客户端连接
        cfd = accept(sfd,(struct sockaddr*)&caddr,&caddrlen);
        if(cfd<0){
            continue;
        }
        //填写客户端信息结构体
        infos[i].connfd = cfd;
        infos[i].addr = caddr;
        //创建子线程
        pthread_create(&tid,NULL,*callback,(void*)&infos[i]);
        //设置子线程分离，防止僵尸线程
        pthread_detach(tid);
        i++;
    }    
    //关闭socket
    close(sfd);
    close(cfd);
    return 0;
}