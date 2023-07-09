#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
using namespace std;

#define SERV_PORT 9527

void sys_err(int ret,const char* str){
    if(ret == -1){
        perror(str);
        exit(-1);
    }
}
int main(int argc, char const *argv[])
{
    cout<<"hello client"<<endl;
    /* code */
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
}
