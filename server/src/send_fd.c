#include <func.h>

void send_fd(int sockpairw,int fd)
{
   struct msghdr  msg;
   bzero(&msg,sizeof(msg));
   char buf1[10]="xiong";
   char buf2[10]="da";
   struct iovec iov[2];
   iov[0].iov_base=buf1;
   iov[0].iov_len=5;
   iov[1].iov_base=buf2;
   iov[1].iov_len=2;
   msg.msg_iov=iov;
   msg.msg_iovlen=2;
   struct cmsghdr *cmsg;
   int len=CMSG_LEN(sizeof(int));
   cmsg=(struct cmsghdr *)calloc(1,len);
   cmsg->cmsg_len=len;
   cmsg->cmsg_level=SOL_SOCKET;
   cmsg->cmsg_type=SCM_RIGHTS;
   msg.msg_control=cmsg;
   msg.msg_controllen=len;

   *(int*)CMSG_DATA(cmsg)=fd;
   sendmsg(sockpairw,&msg,0);
}
void recv_fd(int sockpairr,int *fd)
{
   struct msghdr msg;
   bzero(&msg,sizeof(msg));
   char buf1[10]={0};
   char buf2[10]={0};
   struct iovec iov[2];
   iov[0].iov_base=buf1;
   iov[0].iov_len=5;
   iov[1].iov_base=buf2;
   iov[1].iov_len=2;
   msg.msg_iov=iov;
   msg.msg_iovlen=2;
   struct cmsghdr *cmsg;
   int len=CMSG_LEN(sizeof(int));
   cmsg=(struct cmsghdr *)calloc(1,len);
   cmsg->cmsg_len=len;
   cmsg->cmsg_level=SOL_SOCKET;
   cmsg->cmsg_type=SCM_RIGHTS;
   msg.msg_control=cmsg;
   msg.msg_controllen=len;
   recvmsg(sockpairr,&msg,0);
   *fd=*(int*)CMSG_DATA(cmsg);
}
