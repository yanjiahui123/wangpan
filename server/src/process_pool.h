#include <func.h>
#include <crypt.h>
#define DOWNLOAD "file"
#define MMAPSIZE 100*1024*1024
typedef struct{
    pid_t pid;
    int fds;
    short busy;
}processdata;

typedef struct{
    int datalen;
    char buf[1024];
}data,*pdata;

typedef struct{
    int cmd_len;
    char cmd_buf[1024];
}cmd,*pcmd;
void makechild(processdata* ,int ,int );
void childhandle(int ,int);
int tcp_init(int *,char *,char *);
int send_fd(int,int);
int recv_fd(int ,int *);
int tranfile(int );
int send_n(int sfd,char *p,int len);
int recv_n(int sfd,char *p,int len);
int cmd_handle(int new_fd,struct sockaddr_in,int log_fd);
void do_ls(int new_fd,pcmd pc);
void do_pwd(int new_fd,pcmd pc);
void do_cd(int new_fd,pcmd pc);
void do_puts(int new_fd,pcmd pc);
void do_gets(int new_fd,pcmd pc);
void do_mkdir(int new_fd,pcmd pc);
void do_remove(int new_fd,pcmd pc);
void file_stat(char *res,char *filename);
void change_st_mode(char *perm,struct stat buf);
void change_time_type(char *t);
int recv_file(int new_fd);
int send_file(int new_fd,char *filename);
int is_file_exists(char *filename) ;
int login_verify(int new_fd);
void log_write(int log_fd,char *log_buf);
int mmap_send_file(int new_fd,char *filename);
