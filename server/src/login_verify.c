#include "process_pool.h"
void get_salt(char *salt,char *passwd)
{
    int i,j;
    for(i=0,j=0;passwd[i]&&j!=3;++i)
    {
        if(passwd[i]=='$')
        {
            j++;
        }
    }
    strncpy(salt,passwd,i-1);
}
int login_verify(int new_fd)
{
    struct spwd *sp;
    char usrname[128]={0};
    char passwd[128]={0};
    char salt[128]={0};
    int len=0;
    int flag;
    recv(new_fd,&len,sizeof(int),0);//接收用户名大小
    recv_n(new_fd,usrname,len);//接收用户名

    recv(new_fd,&len,sizeof(int),0);//接收密码长度
    recv_n(new_fd,passwd,len);//接收密码
    
    if((sp=getspnam(usrname))==NULL)
    {
        flag=-1;
        send(new_fd,&flag,sizeof(int),0);//发送用户名不存在标志
        return -1;//退出程序
    }
    //得到salt,用得到的密码做参数
    get_salt(salt,sp->sp_pwdp);
    //进行密码验证
    if(strcmp(sp->sp_pwdp,crypt(passwd,salt))==0)
    {
        flag=0;
        send(new_fd,&flag,sizeof(int),0);
        return 0;
    }
    else
    {
        flag=-1;
        send(new_fd,&flag,sizeof(int),0);
        return -1;
    }
}

