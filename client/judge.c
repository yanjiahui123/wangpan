#include "client.h"

void judge(pcmd pc,pdata pd,int sockfd)
{
    if(strncmp("ls",pc->cmd_buf,2)==0)
    {
        while(1)
        {
            bzero(pd,sizeof(data));
            recv(sockfd,&pd->len,sizeof(int),0);
            if(pd->len==0)
            {
                break;//服务器端发送数据完毕
            }
            recv_n(sockfd,pd->buf,pd->len);
            printf("%s\n",pd->buf);
        }
    }
    else if(strncmp("pwd",pc->cmd_buf,3)==0)
    {
        bzero(pd,sizeof(data));
        recv(sockfd,&pd->len,sizeof(int),0);
        recv_n(sockfd,pd->buf,pd->len);
        printf("%s\n",pd->buf);
    }
    else if(strncmp("cd",pc->cmd_buf,2)==0)
    {
        int flag=0;
        recv(sockfd,&flag,sizeof(int),0);
        if(1==flag)
        {
            printf("this is home\n");
        }
    }
    else if(strncmp("mkdir",pc->cmd_buf,5)==0)
    {
        int flag;
        recv(sockfd,&flag,sizeof(int),0);
        if(0==flag)
        {
            printf("mkdir %s success!\n",pc->cmd_buf+6);
        }
        else
        {
            printf("mkdir %s fail!\n",pc->cmd_buf+6);
        }
    }
    else if(strncmp("remove",pc->cmd_buf,6)==0)
    {
        int flag;
        recv(sockfd,&flag,sizeof(int),0);
        if(0==flag)
        {
            printf("remove %s success!\n",pc->cmd_buf+7);
        }
        else
        {
            printf("remove %s fail!\n",pc->cmd_buf+7);
        }
    }
    else if(strncmp("puts",pc->cmd_buf,4)==0)
    {
       char puts_filename[128]={0};
       sscanf(pc->cmd_buf+5,"%s",puts_filename);
       int flag;
       //如果本地文件存在,则返回0,否则返回1
       if(0==(is_file_exits(puts_filename)))
       {
            struct stat s;
            bzero(&s,sizeof(s));
            stat(puts_filename,&s);//获取文件的属性
            //开火车
            flag=0;
            send_n(sockfd,(char*)&flag,sizeof(int));
            send_file(sockfd,puts_filename);//发送文件
            printf("file %s puts success!\n",puts_filename);
       }
       else
       {
            flag=-1;
            send(sockfd,&flag,sizeof(int),0);
            printf("file %s is not exist\n",puts_filename);
       }
    }
    else if(strncmp("gets",pc->cmd_buf,4)==0)
    {
        int flag;
        //char filename[128]={0};
        //int filelen=0;
        //sscanf(pc->cmd_buf+5,"%s",filename);
        //printf("%s\n",filename);
        //filelen=strlen(filename);
        //send(sockfd,&filelen,sizeof(int),0);//发送文件名大小
        //send_n(sockfd,filename,filelen);//发送文件名
        recv(sockfd,&flag,sizeof(int),0);//接收服务器端文件是否存在的标志
        if(0==flag)//服务器端判断要下载的文件是否存在,存在则会发送0,否则发送-1
        {
            recv_file(sockfd);    
            printf("file %s recv success!\n",pc->cmd_buf+5);
        }
        else
        {
            printf("file %s is not exists!\n",pc->cmd_buf+5);
            return ;
        }
    }
}

