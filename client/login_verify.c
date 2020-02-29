#include "client.h"

int login_verify(int sockfd)
{
    char usrname[128]={0};
    char *passwd;
    int len=0;
    printf("请输入用户名:\n");
    scanf("%s",usrname);

    passwd=getpass("请输入密码:");
    
    len=strlen(usrname);

    send(sockfd,&len,sizeof(int),0);//
    send_n(sockfd,usrname,len);
    
    len=strlen(passwd);
    send(sockfd,&len,sizeof(int),0);
    send_n(sockfd,passwd,len);
    int flag;
    recv(sockfd,&flag,sizeof(int),0);
    if(flag==0)
        return 0;
    else 
        return -1;
}

