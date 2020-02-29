#include "process_pool.h"

void makechild(processdata *pManage,int processNum,int log_fd)
{
    int i;
    pid_t pid;
    int fds[2];
    for(i=0;i<processNum;i++)
    {
        socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
        pid=fork();
        if(!pid)
        {
            close(fds[1]);
            childhandle(fds[0],log_fd);    
        }
        close(fds[0]);
        pManage[i].pid=pid;
        pManage[i].fds=fds[1];
        pManage[i].busy=0;
    }
}

void childhandle(int fds,int log_fd)
{
    int new_fd;
    int flag=1;
    struct sockaddr_in client;
    while(1)
    {
        recv_fd(fds,&new_fd);//子进程等待接收main进程发送过来的客户端请求产生的new_fd
        printf("i am child,get task\n");

        read(fds,&client,sizeof(client));//main进程通过socketpair发送连接客户端的结构体信息过来
//        tranfile(new_fd);//child process send file
        cmd_handle(new_fd,client,log_fd);
        write(fds,&flag,sizeof(int));
    }
}

