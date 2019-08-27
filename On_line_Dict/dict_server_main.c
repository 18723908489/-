/*******************************************************
#	> 文件名: dict_server_main.c
#	> QQ: 652131681
#	> 姓名: 杨志强 
#	> 创建时间: 2019年08月19日 星期一 11时24分05秒
# *********************************************/
#include "dict.h"

int main(int argc,char *argv[])
{
	int n;
	pid_t pid;
	MSG msg;
	sqlite3 *db;
	int acceptfd;
	int sfd;//定义一个文件描述符，是在查看socket函数之后发现返回值是int
	struct sockaddr_in ser; //网络信息填充的结构体,表示服务器地址。
	if(argc<3)
	{
		printf("Usage:%s serverip port.\n",argv[0]);
		return -1;
	}
	if(sqlite3_open(DATABASE,&db)!=SQLITE_OK)//打开数据库
	{
		printf("sqlite3_open:%s\n",sqlite3_errmsg(db));
		return -1;
	}
	if((sfd=socket(AF_INET,SOCK_STREAM,0))<0)//创建套接字
	{
	  perror("fail to socket");
	  return -1;
	}
	memset(&ser,0,sizeof(ser));
	ser.sin_family=AF_INET;//设置协议
	ser.sin_port=htons(atoi(argv[2]));//设置端口
	ser.sin_addr.s_addr=inet_addr(argv[1]);//设置地址
	if(bind(sfd,(struct sockaddr *)&ser,sizeof(ser))<0)//绑定
	{
      perror("fail to bind");
	  return -1;
	}
	if(listen(sfd,BACKLOG)<0)//监听 等待客户端连接
	{
		perror("fail to listen");
		return -1;
	}
	signal(SIGCHLD,SIG_IGN);//处理僵尸进程
	while(1)//1级菜单
	{
		if((acceptfd=accept(sfd,NULL,NULL))<0)//等待连接请求
		{
			perror("fail to accept");
			return -1;
		}
		if((pid=fork())<0)//创建子进程
		{
			perror("fail to fork");
			return -1;
		}else if(pid==0)
		{
			//处理客户端信息
			close(sfd);
			do_client(acceptfd,db);
		} else close(acceptfd);
	}
	close(acceptfd);
	return 0;
}
