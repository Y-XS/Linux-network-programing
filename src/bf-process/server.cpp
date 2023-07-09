#include<iostream>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<wait.h>
using namespace std;

#define SERV_PORT 9999
void sys_err(int ret,const char* str){
    if(ret == -1){
        perror(str);
        exit(-1);
    }
}

void recycle_child_process(int signum){
    pid_t wpid;
    int status;
    while((wpid = waitpid(-1,&status,WNOHANG)) > 0){
        if(WIFEXITED(status))
            cout<<"-------------------wait for child with pid "<<wpid<<" & return "<<WEXITSTATUS(status)<<endl;
    }
}

int main(){
    int lfd,cfd;
    int ret;
    pid_t pid;
    char buf[BUFSIZ],buf_ip[BUFSIZ];
    //阻塞信号
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGCHLD);
    sigprocmask(SIG_BLOCK,&set,NULL);
    
    struct sockaddr_in saddr,caddr;
    socklen_t caddr_len = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(SERV_PORT);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET,SOCK_STREAM,0);
    ret = bind(lfd,(struct sockaddr*)&saddr,sizeof(saddr));
    ret = listen(lfd,128);

    while (1)
    {
        cfd = accept(lfd,(struct sockaddr*)&caddr,&caddr_len);
        cout<<"===== cfd："<<cfd<<endl;

        if(cfd>0){
            pid = fork();
        }else{
            continue;
        }

        if(pid<0){//error
            sys_err(pid,"fork error!");
        }else if(pid==0){//子进程
            //子进程关闭lfd，退出循环
            close(lfd);
            break;
        }else{//父进程
            //注册信号捕捉函数，回收子进程
            struct sigaction act;
            act.sa_handler = recycle_child_process;
            sigemptyset(&act.sa_mask);
            act.sa_flags=0;

            sigaction(SIGCHLD,&act,NULL);
            sigprocmask(SIG_UNBLOCK,&set,NULL);

            //父进程关闭cfd，继续循环监听
            close(cfd);
            continue;
        }
    }
    
    if(pid == 0){
        cout<<"create child process "<<getpid()<<endl;
        cout<<"request from ip: "
            <<inet_ntop(AF_INET,&caddr.sin_addr.s_addr,buf_ip,sizeof(buf_ip))
            <<":"<<ntohs(caddr.sin_port)<<endl;
        while(1){
            //子进程业务函数
            int n = read(cfd,buf,1024);
            if(n==0){
                close(cfd);
                exit(1);
            }
            for(int i=0;i<n;i++){
                buf[i] = toupper(buf[i]);
            }
            write(cfd,buf,n);
            write(STDOUT_FILENO,buf,n);
            // cout<<buf<<flush;
        }
    }

    close(lfd);
    close(cfd);
    return 0;
}