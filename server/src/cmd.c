#include "process_pool.h"
//实现ftp的ls的功能,这里利用LINUX的接口实现出ls -ll的样子
void change_time_type(char *t)
{
    char *p;
    p=t+strlen(t)-1;
    while(*p!=':')
    {
        p--;
    }
    *p='\0';
}

void change_st_mode(char *perm,struct stat buf)
{
    if(S_ISDIR(buf.st_mode))
    {
        perm[0]='d';
    }
    if(S_ISREG(buf.st_mode))
    {
        perm[0]='-';
    }
    else
    {
        perm[0]='*';
    }
    if((buf.st_mode & S_IRUSR) == S_IRUSR)
    {
        perm[1]='r';
    }
    if((buf.st_mode & S_IWUSR) == S_IWUSR)
    {
        perm[2]='w';
    }
    if((buf.st_mode & S_IXUSR) == S_IXUSR)
    {
        perm[3]='x';
    }
    if((buf.st_mode & S_IRGRP) == S_IRGRP)
    {
        perm[4]='r';
    }
    if((buf.st_mode & S_IWGRP) == S_IWGRP)
    {
        perm[5]='w';
    }
    if((buf.st_mode & S_IXGRP) == S_IXGRP)
    {
        perm[6]='x';
    }
    if((buf.st_mode & S_IROTH) == S_IROTH)
    {
        perm[7]='r';
    }
    if((buf.st_mode & S_IWOTH) == S_IWOTH)
    {
        perm[8]='w';
    }
    if((buf.st_mode & S_IXOTH) == S_IXOTH)
    {
        perm[9]='x';
    }
    perm[10]='\0';
}
void file_stat(char *res,char *filename)
{
    struct stat buf;
    memset(&buf,0,sizeof(buf));
    int ret=stat(filename,&buf);
    if(-1==ret)
    {
        perror("stat");
        return ;
    }
    char t[128];
    strcpy(t,ctime(&buf.st_mtime));
    change_time_type(t);
    char perm[10];
    memset(perm,'-',sizeof(perm));
    change_st_mode(perm,buf);
    sprintf(res,"%s %ld %s %s %6ld %s %s",perm,buf.st_nlink,getpwuid(buf.st_uid)->pw_name,getgrgid(buf.st_gid)->gr_name,buf.st_size,t+4,filename);
    return ;
}
void do_ls(int new_fd,pcmd pc)
{
    char *pwd=getcwd(NULL,0);
    DIR *dir=NULL;
    dir=opendir(pwd);
    if(NULL==dir)
    {
        perror("opendir");
        return ;
    }
    struct dirent *q=NULL;
    off_t I;
    while((q=readdir(dir))!=NULL)
    {
        if((strcmp(q->d_name,".")!=0) && (strcmp(q->d_name,"..")!=0))
        {
            bzero(pc,sizeof(cmd));
            file_stat(pc->cmd_buf,q->d_name);//传入数组,获取ls -ll命令能显示的信息
            pc->cmd_len=strlen(pc->cmd_buf);
            if(q->d_type==4)
            {
                pc->cmd_buf[0]='d';
            }
            send(new_fd,&pc->cmd_len,sizeof(int),0);//送大小
            send_n(new_fd,pc->cmd_buf,pc->cmd_len);//送内容过去
        }
    }
    int end=0;
    send(new_fd,&end,sizeof(int),0);
}

void do_pwd(int new_fd,pcmd pc)
{
    char pwd[128]={0};
    getcwd(pwd,sizeof(pwd));
    int pwd_len=strlen(pwd);
    send(new_fd,&pwd_len,sizeof(int),0);
    send_n(new_fd,pwd,pwd_len);
}

void do_cd(int new_fd,pcmd pc)
{
    char *pwd=getcwd(NULL,0);
    int flag=0;
    char dir[128]={0};
    bzero(dir,sizeof(dir));
    sscanf(pc->cmd_buf+3,"%s",dir);//截取要转移到的目录,所以+3,忽略cd和一个空格
    if(strcmp("/home/yijh/process_pool/wangpan",pwd)==0)
    {
        if(strncmp("..",dir,2)==0)
        {
            flag=1;
            send(new_fd,&flag,sizeof(int),0);
            return ;
        }
    }
    send(new_fd,&flag,sizeof(int),0);
    chdir(dir);
}
void do_puts(int new_fd,pcmd pc)
{
   int flag;
   recv(new_fd,&flag,sizeof(int),0);
   if(0==flag)//对面确定要发送的文件存在会先发一个0
   {
        recv_file(new_fd);
   }
   else//客户端发送-1过来 
   {
       return ;
   }
}
void do_gets(int new_fd,pcmd pc)
{
    //pc里就存了文件名。。。。
    //不需要重新接收文件名
    char filename[128]={0};
    sscanf(pc->cmd_buf+5,"%s",filename);
    int flag,ret;
    //recv(new_fd,&filelen,sizeof(int),0);//获取文件名大小
    //recv_n(new_fd,filename,filelen);//获取文件名
    if(0==is_file_exists(filename))//判断服务器是否存在客户端要下载的文件
    {
        struct stat s;
        bzero(&s,sizeof(s));
        ret=stat(filename,&s);
        if(-1==ret)
        {
            perror("stat");
            return ;
        }
        if(s.st_size<MMAPSIZE)
        {
            flag=0;
            send(new_fd,&flag,sizeof(int),0);//发送标志过去
            send_file(new_fd,filename);//发送文件
        }
        else{
            flag=0;
            send(new_fd,&flag,sizeof(int),0);//发送标志过去
            mmap_send_file(new_fd,filename);
        }
    }
    else
    {
        flag=-1;
        send(new_fd,&flag,sizeof(int),0);//发送文件不存在标志
    }
}
void do_mkdir(int new_fd,pcmd pc)
{
    char dirname[128]={0};
    bzero(dirname,sizeof(dirname));
    sscanf(pc->cmd_buf+6,"%s",dirname);
    int flag=mkdir(dirname,0777);
    send(new_fd,&flag,sizeof(int),0);//成功返回0,失败返回1
}
void do_remove(int new_fd,pcmd pc)
{
    char dirname[128]={0};
    bzero(dirname,sizeof(dirname));
    sscanf(pc->cmd_buf+7,"%s",dirname);
    int flag=rmdir(dirname);
    send(new_fd,&flag,sizeof(int),0);
}



int cmd_handle(int new_fd,struct sockaddr_in client,int log_fd)
{
    //获取信号量对日志文件进行读写操作
    int semid=semget(1000,1,IPC_CREAT|0600);//main进程已经创建，这里获取
    struct sembuf sp,sv;
    bzero(&sp,sizeof(sp));
    bzero(&sv,sizeof(sv));
    sp.sem_num=0;
    sp.sem_op=1;
    sp.sem_flg=SEM_UNDO;

    sv.sem_num=0;
    sv.sem_op=-1;
    sv.sem_flg=SEM_UNDO;

    char log_buf[1048]={0};
    

    int recv_ret;
    pcmd pc=(pcmd)calloc(1,sizeof(cmd));
    int ret;
    ret=login_verify(new_fd);
    if(-1==ret)
    {
        close(new_fd);
        free(pc);
        pc=NULL;
        printf("client connect reject!\n");
        bzero(log_buf,sizeof(log_buf));
        sprintf(log_buf,"client IP=%s,port=%d\n%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),"client connect reject!");
        semop(semid,&sp,1);
        log_write(log_fd,log_buf);
        semop(semid,&sv,1);
        return 0;
    }
    
    bzero(log_buf,sizeof(log_buf));
    sprintf(log_buf,"client IP=%s,port=%d\n%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),"client connect success!");
    semop(semid,&sp,1);
    log_write(log_fd,log_buf);
    semop(semid,&sv,1);


    while(1)
    {
        bzero(pc,sizeof(cmd));
        recv_ret=recv(new_fd,&pc->cmd_len,sizeof(int),0);
        if(recv_ret==0 || pc->cmd_len==0)
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\n%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),"client exit!");
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            
            printf("client exit\n");
            free(pc);
            pc=NULL;
            return 0;
        }
        recv_n(new_fd,pc->cmd_buf,pc->cmd_len);
        if(0==strncmp("ls",pc->cmd_buf,2))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_ls(new_fd,pc);
        }
        else if(0==strncmp("pwd",pc->cmd_buf,3))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_pwd(new_fd,pc);
        }
        else if(0==strncmp("cd",pc->cmd_buf,2))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_cd(new_fd,pc);
        }
        else if(0==strncmp("puts",pc->cmd_buf,4))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_puts(new_fd,pc);
        }  
        else if(0==strncmp("gets",pc->cmd_buf,4))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_gets(new_fd,pc);
        }
        else if(0==strncmp("mkdir",pc->cmd_buf,5))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_mkdir(new_fd,pc);
        }   
        else if(0==strncmp("remove",pc->cmd_buf,6))
        {
            bzero(log_buf,sizeof(log_buf));
            sprintf(log_buf,"client IP=%s,port=%d\ncmd:%s\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port),pc->cmd_buf);
            semop(semid,&sp,1);
            log_write(log_fd,log_buf);
            semop(semid,&sv,1);
            do_remove(new_fd,pc);
        }
    }
    
    return 0;
}

