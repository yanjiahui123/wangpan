#include <func.h>
#include <crypt.h>
typedef struct{
    int len;
    char buf[2000];
}data,*pdata;

typedef struct{
    int cmd_len;
    char cmd_buf[2000];
}cmd,*pcmd;

void send_n(int new_fd,char *buf,int len);
void recv_n(int new_fd,char *buf,int len);
void deal_cmd(char *cmd);
int is_file_exits(char *puts_filename);
int send_file(int new_fd,char *filename);
int recv_file(int new_fd);
void judge(pcmd pc,pdata pd,int sockfd);   
int login_verify(int sockfd);
