#include "client.h"
int is_file_exits(char *filename)
{
    char *pwd=getcwd(NULL,0);
    DIR *dir=opendir(pwd);
    if(NULL==dir)
    {
        return -2;//意味着失败
    }
    struct dirent*q;
    while((q=readdir(dir))!=NULL)
    {
        if(strcmp(q->d_name,filename)==0)
        {
            return 0;
        }
    }
    return -1;
}

int is_file_exits_tmp(char *filename)
{
    char *pwd=getcwd(NULL,0);
    DIR *dir=opendir(pwd);
    if(NULL==dir)
    {
        perror("opendir");
        return -2;
    }
    char tmp[128]={0};
    strcpy(tmp,filename);
    strcat(tmp,".mh");
    printf("tmp file %s \n",tmp);
    struct dirent *q=NULL;
    while((q=readdir(dir))!=NULL)
    {
        if(strcmp(tmp,q->d_name)==0)
        {
            return 1;//存在临时文件
        }
    }
    return -1;
}
int recv_file(int new_fd)
{
    char file_name[128]={0};
    int file_len=0;
    recv_n(new_fd,(char*)&file_len,sizeof(int));
    recv_n(new_fd,file_name,file_len);//接收文件名
    if(1==(is_file_exits_tmp(file_name)))
    {
        char file_tmp[128]={0};
        sprintf(file_tmp,"%s%s",file_name,".mh");//.mh是临时文件
        int fd=open(file_tmp,O_APPEND|O_RDWR);
        if(-1==fd)
        {
            perror("tmp_open");
            return -1;
        }
        struct stat s;
        bzero(&s,sizeof(s));
        fstat(fd,&s);
        send(new_fd,&s.st_size,sizeof(int),0);//发送已存在的文件大小
        printf("file already accepted %ld, keep recving!\n",s.st_size);
        data d;
        while(1)
        {
            bzero(&d,sizeof(d));
            recv(new_fd,&d.len,sizeof(int),0);//接收文件内容大小
            if(0==d.len)
            {
                break;
            }
            recv_n(new_fd,d.buf,d.len);//接收文件内容
            write(fd,d.buf,d.len);
        }
        close(fd);
        char cmd[128]={0};
        //给临时文件改名
        sprintf(cmd,"%s %s %s","mv",file_tmp,file_name);
        system(cmd);
        printf("file %s recv success!\n",file_name);
    }
    else
    {
        char file_tmp[1024]={0};
        sprintf(file_tmp,"%s%s",file_name,".mh");
        printf("file_tmp %s\n",file_tmp);
        int fd=open(file_tmp,O_WRONLY|O_CREAT,0666);
        
        printf("file_tmp=%s\n",file_tmp);
        if(-1==fd)
        {
            perror("open");
            printf("file %s recv failed\n",file_name);
            return -1;
        }
        printf("recving file\n");
        int file_len=0;
        int ret;
        send(new_fd,&file_len,sizeof(int),0);//发送空文件的大小
        data d;
        while(1)
        {
            bzero(&d,sizeof(d));
            ret=recv(new_fd,&d.len,sizeof(int),0);
            ERROR_CHECK(ret,-1,"recv");
            if(0==d.len)
            {
                break;
            }
            recv_n(new_fd,d.buf,d.len);
            ret=write(fd,d.buf,d.len);
            ERROR_CHECK(ret,-1,"write");
        }
        close(fd);
        char cmd[2024]={0};
        sprintf(cmd,"%s %s %s","mv",file_tmp,file_name);
        system(cmd);
        printf("file %s recv success!\n",file_name);
    }
    return 0;
}
int send_file(int new_fd,char *filename)
{
    int file_len=0;
    file_len=strlen(filename);
    send_n(new_fd,(char*)&file_len,sizeof(int));//发送文件名长度
    send_n(new_fd,filename,file_len);//发送文件名
    int fd=open(filename,O_RDONLY);//读取文件
    ERROR_CHECK(fd,-1,"open");
    printf("sending ...\n");
    data d;
    while(bzero(&d,sizeof(d)),(d.len=read(fd,d.buf,sizeof(d.buf)))>0)
    {
        send_n(new_fd,(char*)&d.len,sizeof(int));
        send_n(new_fd,d.buf,d.len);
    }
    int endflag=0;
    send_n(new_fd,(char*)&endflag,sizeof(int));
    printf("send %s success\n",filename);
    close(fd);
    return 0;
}
