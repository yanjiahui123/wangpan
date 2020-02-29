#include "process_pool.h"

int tranfile(int new_fd)
{
    data t;
    bzero(&t,sizeof(t));
    strcpy(t.buf,DOWNLOAD);
    t.datalen=strlen(DOWNLOAD);
    
    //send file name
    send(new_fd,&t,4+t.datalen,0);
    //send file size
    struct stat buf;
    stat(DOWNLOAD,&buf);
    t.datalen=sizeof(buf.st_size);
    memcpy(t.buf,&buf.st_size,t.datalen);
    send(new_fd,&t,4+t.datalen,0);
    //send file contain
    int fd=open(DOWNLOAD,O_RDONLY);
    
    while(t.datalen=read(fd,t.buf,sizeof(t.buf)))
    {
        send(new_fd,&t,4+t.datalen,0);
    }
    //send file over
    send(new_fd,&t,4+t.datalen,0);
    close(new_fd);
    return 0;
}

