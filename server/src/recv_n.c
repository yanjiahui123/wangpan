#include "process_pool.h"

int send_n(int new_fd,char *buf,int len)
{
    int ret;
    int total=0;
    while(total<len)
    {
        ret=send(new_fd,buf+total,len-total,0);
        if(-1==ret)
        {
            return -1;
        }
        total+=ret;
    }
    return 0;
}

int recv_n(int new_fd,char *buf,int len)
{
    int ret;
    int total=0;
    while(total<len)
    {
        ret=recv(new_fd,buf+total,len-total,0);
        total+=ret;
    }
    return 0;
}

