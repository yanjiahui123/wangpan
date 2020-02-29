#include "client.h"

int main(int argc,char *argv[])
{
    ARGS_CHECK(argc,3);
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(-1==sockfd)
    {
        perror("socket");
        return -1;
    }
    printf("sockfd=%d\n",sockfd);
    struct sockaddr_in ser;
    memset(&ser,0,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_port=htons(atoi(argv[2]));
    ser.sin_addr.s_addr=inet_addr(argv[1]);
    int ret;
    ret=connect(sockfd,(struct sockaddr*)&ser,sizeof(struct sockaddr));
    if(-1==ret)
    {
        perror("connect");
        return -1;
    }

    //发送用户名和密码
    ret=login_verify(sockfd);
    if(-1==ret)
    {
        printf("verify fail!\n");
        close(sockfd);
        return -1;
    }
    printf("verify success!\n");

    char buf[128]={0};
    fd_set rdset;
    pdata pd=(pdata)calloc(1,sizeof(data));
    pcmd pc=(pcmd)calloc(1,sizeof(cmd));
    int read_len=0;
    while(1)
    {
        FD_ZERO(&rdset);
        FD_SET(sockfd,&rdset);
        FD_SET(STDIN_FILENO,&rdset);
        ret=select(sockfd+1,&rdset,NULL,NULL,NULL);
        if(ret>0)
        {
            if(FD_ISSET(STDIN_FILENO,&rdset))
            {
                bzero(pc,sizeof(cmd));
                fflush(stdin);
                read_len=read(STDIN_FILENO,pc->cmd_buf,sizeof(pc->cmd_buf));
                if(0==read_len)
                {
                    printf("I will close\n");
                    break;
                }
                deal_cmd(pc->cmd_buf);//处理字符串,把多余的空格去掉
                pc->cmd_len=strlen(pc->cmd_buf);
                send(sockfd,&pc->cmd_len,sizeof(int),0);
                send_n(sockfd,pc->cmd_buf,pc->cmd_len);
                //判断相应的字符进行处理
                judge(pc,pd,sockfd);
                //send(sockfd,buf,strlen(buf)-1,0);
            }
            if(FD_ISSET(sockfd,&rdset))
            {
                memset(buf,0,sizeof(buf));
                ret=recv(sockfd,buf,sizeof(buf)-1,0);
                if(0==ret)
                {
                    printf("server close,byebye\n");
                    break;
                }
                printf("%s\n",buf);
            }
        }
    }
    close(sockfd);
    return 0;
}
