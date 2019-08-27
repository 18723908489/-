/*******************************************************
#	> 文件名: dict_server.c
#	> QQ: 652131681
#	> 姓名: 杨志强 
#	> 创建时间: 2019年08月19日 星期一 09时49分11秒
# *********************************************/
#include "dict.h"

//acceptfd：文件描述符 msg:消息队列结构体 db:数据库
void do_register(int acceptfd,MSG *msg,sqlite3 *db)//注册
{
   char *errmsg;//用于保存出错消息
   char sql[1024];//sql语句
   sprintf(sql,"insert into user values('%s','%s');",msg->name,msg->data);//插入数据
   if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)//执行sql语句
   {
	   printf("sqlite3_exec 1 :%s\n",errmsg);
	   strcpy(msg->data,"user name already exist.");
   }else
   {
	   printf("注册成功!\n");
	   strcpy(msg->data,"OK");
   }
   if(send(acceptfd,msg,sizeof(msg),0)<0)//发送消息失败
   {
	   perror("faile to send");
	   return;
   }
   return ;
}

int login(int acceptfd,MSG *msg,sqlite3 *db)//登录
{
	char *errmsg;//用于保存出错消息
	char sql[1024];//sql语句
	int nrow;//数据库函数执行后得到记录的数目
	int ncloumn;//数据库函数执行后得到字段的数目
	char **resultp;
	sprintf(sql,"select * from user where name = '%s' and passwd='%s';",msg->name,msg->data);
    if((sqlite3_get_table(db,sql,&resultp,&nrow,&ncloumn,&errmsg))!=SQLITE_OK)
	{
		printf("sqlite3_exec 2 :%s\n",errmsg);
		return -1;
	}else 
	{
		printf("登录成功!\n");
	}
	if(nrow==1)//查询成功
	{
		strcpy(msg->data,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		return 1;
	}
	if(nrow==0)//出错
	{
		strcpy(msg->data,"用户/密码 出错!");
		send(acceptfd,msg,sizeof(msg),0);
		return -1;
	}
	return 0;
}

int serchword(int acceptfd,MSG *msg,char word[])//查找单词
{
	FILE *fp;//定义文件指针
	int len=0,i=0;
	char temp[512]={0};
	int result;
	char *p;
	if((fp=fopen("dict.txt","r"))==NULL)//打开文件失败
	{
		perror("fail to open!\n");
		strcpy(msg->data,"Failed to open dict.txt\n");//保存文件打开失败信息
		send(acceptfd,msg,sizeof(MSG),0);//将消息发送给客户端
		return -1;
	}
     len=strlen(word);
	//从文件中查找单词
	while(fgets(temp,sizeof(temp),fp)!=NULL)//从fd中读取512字节放在temp中
	{
		result=strncmp(temp,word,len);//输入的单词与查询的单词进行比较,并将结果保存result中
	 if(result>0)
	 {
		 break;
	 }
	 if(result<0||temp[len]!=' ')
	 {
		 continue;
	 }
     if(result==0)
	   {
		   //找到该单词
			p=temp+len;
			while(*p==' ') //将指针定义到注释前面一个位置
				p++;
			//找到注释,跳过空格
			strcpy(msg->data,p);//保存注释
			send(acceptfd,msg,sizeof(MSG),0);//将结果发送给客户端
			fclose(fp);//关闭文件
			return 1;
	   }else continue;
	}
	strcpy(msg->data,"没有找到该单词！");
	fclose(fp);
	return -1;
}

int get_data(char *date)//获取系统时间进行格式转换
{
	time_t t;//定义时间
	struct tm *tp;//定义时间结构体
	time(&t);//获得时间
	tp=localtime(&t);//进行时间转换
	sprintf(date,"%d-%d-%d %d:%d:%d",tp->tm_year+1990,tp->tm_mon+1,tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);//将时间保存在date中
	return 0;
}

int query(int acceptfd,MSG *msg,sqlite3 *db)//查询
{
	char word[64];//保存单词
	int found=0;//表示有没有找到单词
	char data[128]={0};//保存注释
	char sql[128]={0};//保存sql语句
	char *errmsg;
	//取出单词
	strcpy(word,msg->data);
	found=serchword(acceptfd,msg,word);
	//找到单词，将用户名和时间、单词插入到历史记录表中去
	if(found==1)
	{
		//获取系统时间
		get_data(data);
	    sprintf(sql,"insert into record values('%s','%s','%s');",msg->name,data,word);//插入到record表中
		 if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)//插入失败
		 {
			 printf(" 3 %s\n",errmsg);
			 return -1;
		 }
	}else
	{
		strcpy(msg->data,"没有找到");
		send(acceptfd,msg,sizeof(MSG),0);//将结果发送给客户端
	}
	fflush(stdout);
	return 0;
}
int history_callback(void *arg,int f_num,char**f_value,char ** f_name)//回调函数
{
	int acceptfd;
	MSG msg;
	acceptfd=*((int*)arg);//类型转换
	sprintf(msg.data,"%s , %s , %s",f_value[0],f_value[1],f_value[2]);//保存数据
	if(0>send(acceptfd,&msg,sizeof(MSG),0))//发送数据给客户端
	{
		perror("history_callback:");
		return -1;
	}
	return 0;
}

int history(int acceptfd,MSG *msg,sqlite3 *db)//查询历史记录
{
   char sql[128]={0};
   char *errmsg;
   sprintf(sql,"select * from record where name = '%s';",msg->name);//保存查询结果
   if(sqlite3_exec(db,sql,history_callback,(void *)&acceptfd,&errmsg)!=SQLITE_OK)
   {
	   printf("sqlite3_exec 4 :%s",errmsg);
   }
   //全部查询完成，发送结束标志，让客户端停止接收
   msg->data[0]='\0';
   if(0>send(acceptfd,msg,sizeof(MSG),0))//查询完成给客户端发送消息
   {
	   printf("发送出错!\n");
	   return -1;
   }
   return 1;
}

int delete_callback(void *arg,int f_num,char**f_value,char ** f_name)
{
   
	int acceptfd;
	MSG msg;
	acceptfd=*((int*)arg);//类型转换
	sprintf(msg.data,"%s , %s , %s",f_value[0],f_value[1],f_value[2]);//保存数据
	if(0>send(acceptfd,&msg,sizeof(MSG),0))//发送数据给客户端
	{
		 perror("delete_callback:");
		 return -1;
	}
	return 1;
}

int do_delete(int acceptfd,MSG *msg,sqlite3 *db)
{
   char sql[128]={0};
   char *errmsg;
   sprintf(sql,"delete from record where name = '%s';",msg->name);//sql删除语句
   if(sqlite3_exec(db,sql,delete_callback,(void *)&acceptfd,&errmsg)!=SQLITE_OK)
   {
	   printf("sqlite3_exec 4 :%s",errmsg);
   return -1;
   }
   strcpy(msg->data,"OK");
   if(0>send(acceptfd,msg,sizeof(MSG),0))
   {
	   perror("delete  send");
	   return -1;
   }
   return 1;
}

int do_client(int acceptfd,sqlite3 *db)//处理菜单
{
   MSG msg;
   while(recv(acceptfd,&msg,sizeof(msg),0)>0)
   { switch(msg.type)
	   {
		   case R:
			   do_register(acceptfd,&msg,db);
			   break;
		   case L:
			   login(acceptfd,&msg,db);
			   break;
		   case Q:
			   if(query(acceptfd,&msg,db)==1)
			   {
				   printf("查询成功");
			   }
			   break;
		   case H:
			       history(acceptfd,&msg,db);
			   break;
		   case D:
			    if(do_delete(acceptfd,&msg,db)==1)
					printf("删除成功!\n");
					break;
		   default:
			   printf("输入有误！\n");
	   }
   }
	   printf("客户端退出!\n");
	   close(acceptfd);
	   exit(0);
	   return 0;
}

