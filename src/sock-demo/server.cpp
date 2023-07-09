#include<iostream>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
using namespace std;

void sys_err(int ret,const char* str){
    if(ret == -1){
        perror(str);
        exit(-1);
    }
}
int main(){
    int sfd,cfd;
    int ret=0;
    socklen_t caddrlen;
    char buf[BUFSIZ], client_ip_buf[1024];
    //填写地址信息
    struct sockaddr_in saddr,caddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9527);
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
    return 0;
}