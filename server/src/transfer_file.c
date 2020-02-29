#include "process_pool.h"

int recv_file(int new_fd)
{
    char file_name[128]={0};
    int file_len=0;
    recv_n(new_fd,(char*)&file_len,sizeof(int));//接收文件长度
    recv_n(new_fd,file_name,file_len);//接收文件名
    int fd=open(file_name,O_WRONLY|O_CREAT,0666);//打开文件或者创建文件
    if(-1==fd)
    {
        perror("open");
        printf("file %s recv failed\n",file_name);
        return -1;
    }
    printf("recving file\n");
    data d;
    //循环接收文件内容
    while(1)
    {
        bzero(&d,sizeof(d));
        recv_n(new_fd,(char*)&d.datalen,sizeof(int));//接收数据大小
        if(d.datalen==0)//接收到对面发送的0就表示对面发送完毕了
        {
            break;
        }
        recv_n(new_fd,d.buf,d.datalen);//接收数据内容
        write(fd,d.buf,d.datalen);//写入文件
    }
    close(fd);//关闭文件
    printf("file %s recv success\n",file_name);
    return 0;
}

int is_file_exists(char *filename)
{
    char *pwd=getcwd(NULL,0);
    DIR *dir=opendir(pwd);
    if(NULL==dir)
    {
        perror("opendir");
        return -2;
    }
    struct dirent *q;
    while((q=readdir(dir))!=NULL)
    {
        if(strcmp(q->d_name,filename)==0)
        {
            return 0;
        }
    }
    return -1;
}

int send_file(int new_fd,char * filename)
{
    int filelen=0;
    filelen=strlen(filename);
    send_n(new_fd,(char*)&filelen,sizeof(int));//发送文件名大小
    send_n(new_fd,filename,filelen);//发送文件名

    int fd=open(filename,O_RDONLY);//打开本地文件进行读取
    ERROR_CHECK(fd,-1,"open");
    
    int file_offset_len;
    recv(new_fd,&file_offset_len,sizeof(int),MSG_WAITALL);
    int ret=lseek(fd,file_offset_len,SEEK_SET);
    if(-1==ret)
    {
        perror("open");
        printf("send %s fail!\n",filename);
        return -1;
    }
    printf("sending ...\n");
    data d;
    while(bzero(&d,sizeof(d)),(d.datalen=read(fd,d.buf,sizeof(d.buf)))>0)
    {
        ret=send(new_fd,&d.datalen,sizeof(int),0);
        if(-1==ret)
        {
            return -1;
        }
        ret=send_n(new_fd,d.buf,d.datalen);
        if(-1==ret)
        {
            return -1;
        }
    }
    int endflag=0;
    send(new_fd,&endflag,sizeof(int),0);
    printf("send %s success!\n",filename);
    close(fd);
    return 0;
}
