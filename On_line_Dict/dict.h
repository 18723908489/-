/*******************************************************
#	> 文件名: dict.h
#	> QQ: 652131681
#	> 姓名: 杨志强 
#	> 创建时间: 2019年08月19日 星期一 09时43分49秒
# *********************************************/
#ifndef _DICT_H_
#define _DICT_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sqlite3.h>
#include<signal.h>
#include<time.h>

#define R 1 //注册
#define L 2 //登录
#define Q 3 //查询
#define H 4 //查询历史
#define D 5 //删除
#define N 20//大小
#define DATABASE "dict.db"
#define BACKLOG 5
typedef struct //定义消息通信的信息结构体
{
   int type;
   char name[N];
   char data[256];
}MSG;

void do_register(int acceptfd,MSG *msg,sqlite3 *db); //需要文件描述符，传输结构体
int history(int acceptfd,MSG *msg,sqlite3 *db);//数据库操作
int history_callback(void *arg,int f_num,char**f_value,char ** f_name);//回调函数
int query(int acceptfd,MSG *msg,sqlite3 *db);//查询
int get_data(char *date);//获取系统时间进行格式转换
int serchword(int acceptfd,MSG *msg,char word[]);//查找单词
int login(int acceptfd,MSG *msg,sqlite3 *db);//登录
int do_client(int acceptfd,sqlite3 *db);//处理菜单
int do_delete(int accept,MSG *msg,sqlite3 *db);//删除
#endif
