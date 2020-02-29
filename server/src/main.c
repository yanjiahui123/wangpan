#include "process_pool.h"
int exitfds[2];
static key_t semid;
void sigfunc(int signum)
{
    write(exitfds[1],&signum,sizeof(int));
}
int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,4);// ip port processNUm

    int ret;
    semid=semget(1000,1,IPC_CREAT|0600);
    ERROR_CHECK(semid,-1,"semget");
    ret=semctl(semid,0,SETVAL,1);
    ERROR_CHECK(ret,-1,"semctl");
    struct sembuf sp,sv;
    bzero(&sp,sizeof(sp));
    bzero(&sv,sizeof(sv));
    sp.sem_num=0;
    sp.sem_op=1;
    sp.sem_flg=SEM_UNDO;

    sv.sem_num=0;
    sv.sem_op=-1;
    sv.sem_flg=SEM_UNDO;

    int log_fd=open("../log/log",O_CREAT|O_WRONLY|O_APPEND,0666);//打开日志文件记录
    if(log_fd==-1)
    {
        printf("log can not find\n");
        perror("open");
        return -1;
    }

    pipe(exitfds);
    signal(SIGUSR1,sigfunc);
    int processNum=atoi(argv[3]);
    processdata * pManage=( processdata *)calloc(processNum,sizeof(processdata));
    //信号量pv操作对log文件操作
    makechild(pManage,processNum,log_fd);//create child process
    int sockfd;
    tcp_init(&sockfd,argv[1],argv[2]);
    
    int epfd=epoll_create(1);
    struct epoll_event event,*evs;
    evs=(struct epoll_event*)calloc(processNum+2,sizeof(struct epoll_event));
    event.events=EPOLLIN;
    event.data.fd=sockfd;
    ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&event);
    ERROR_CHECK(ret,-1,"epoll_ctl");


    int readyNum,i,new_fd,j;
    int flag=1;
    for(i=0;i<processNum;i++)
    {
        event.data.fd=pManage[i].fds;
        epoll_ctl(epfd,EPOLL_CTL_ADD,pManage[i].fds,&event);
    }

    struct sockaddr_in client;
    bzero(&client,sizeof(client));
    socklen_t addrlen=sizeof(client);
    char log_buf[128]={0};
    while(1)
    {
        readyNum=epoll_wait(epfd,evs,processNum+2,-1);
        
        for(i=0;i<readyNum;i++)
        {
            if(sockfd==evs[i].data.fd)
            {
                new_fd=accept(sockfd,(struct sockaddr*)&client,&addrlen);
                ERROR_CHECK(new_fd,-1,"accept");
                
                bzero(log_buf,sizeof(log_buf));
                sprintf(log_buf,"client IP=%s,port=%d\n%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),"client connect");
                semop(semid,&sp,1);//多个进程要来对日志文件写操作，所以必须互斥访问
                log_write(log_fd,log_buf);
                semop(semid,&sv,1);
                
                printf("client ip=%s,port=%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                for(j=0;j<processNum;j++)
                {
                    if(0==pManage[j].busy)
                    {
                        send_fd(pManage[j].fds,new_fd);
                        write(pManage[j].fds,&client,sizeof(client));//用进程间的管道把客户端信息发送过去
                        pManage[j].busy=1;
                        printf("%d is busy\n",pManage[j].pid);//new_fd已经发送给空闲子进程,main进程不再需要new_fd
                        close(new_fd);
                        break;
                    }
                }
            }
            for(j=0;j<processNum;j++)
            { 
                if(pManage[j].fds==evs[j].data.fd)
                {
                    read(pManage[j].fds,&flag,sizeof(int));
                    pManage[j].busy=0;
                    printf("%d is not busy\n",pManage[j].pid);
                    break;
                }              

            }
            //当进入后代表要退出了
            if(exitfds[0]==evs[i].data.fd)
            {
                int flag;
                event.events=EPOLLIN;
                event.data.fd=sockfd;
                epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&event);
                close(sockfd);
                for(j=0;j<processNum;j++)
                {
                    send(new_fd,&flag,-1,0);
                }
                for(j=0;j<processNum;j++)
                {
                    wait(NULL);
                    printf("child exit success\n");
                }
                exit(0);
            }
        }
    }
    return 0;
}

