#include "process_pool.h"

int mmap_send_file(int new_fd,char *filename)
{
    printf("mmap send file %s\n",filename);
    int filelen=strlen(filename);
    send(new_fd,&filelen,sizeof(int),0);//发送文件名长度
    send_n(new_fd,filename,filelen);//发送文件名内容
    int fd=open(filename,O_RDONLY);
    if(-1==fd)
    {
        printf("send file %s fail!\n",filename);
        perror("open");
        return -1;
    }
    int file_offset_len;//文件偏移量
    recv(new_fd,&file_offset_len,sizeof(int),0);//确定文件要哪里开始发，断点续传
    struct stat s;
    bzero(&s,sizeof(s));
    fstat(fd,&s);
    int file_size=s.st_size;
    char *file_point;
    file_point=(char *)mmap(NULL,file_size,PROT_READ,MAP_SHARED,fd,0);
    if(file_point==MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }
    data d;
    bzero(&d,sizeof(d));
    d.datalen=file_size-file_offset_len;//这里算出还需要发多少大小文件
    int tmp=sizeof(d.buf);//切割火车车厢,每次发一个整块，最后一个发一小块
    int ret;
    while(d.datalen>0)
    {
        if(d.datalen>tmp)
        {
            ret=send(new_fd,&tmp,sizeof(int),0);
            if(ret==-1)
            {
                return -1;
            }
            ret=send_n(new_fd,file_point+file_offset_len,tmp);
            if(-1==ret)
            {
                return -1;
            }
            d.datalen-=tmp;
            file_offset_len+=tmp;
        }
        else
        {
            tmp = file_size-file_offset_len;
            ret=send(new_fd,&tmp,sizeof(int),0);
            if(-1==ret)
            {
                return -1;
            }
            ret=send_n(new_fd,file_point+file_offset_len,tmp);
            if(-1==ret)
            {
                return -1;
            }
            d.datalen-=tmp;
            file_offset_len+=tmp;
        }
    }
    //发送结束标志
    int endflag=0;
    send(new_fd,&endflag,sizeof(int),0);
    printf("mmap send %s success!\n",filename);
    close(fd);
    return 0;
}

