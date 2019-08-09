#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "head.h"
#include "work_que.h"
#define STR_LEN 8  //随机数长度

typedef struct{
    Que_t que;
    pthread_cond_t cond;
    pthread_t *pthid;
    int threadNum;
    int startFlag;
}Factory_t, *pFactory_t;

typedef struct{
    int dataLen;  //控制数据，火车头，代表后面存储的数据长度
    int command_code;  //区分用户输入的命令，六种命令，从1开始编号
    char buf[1000];
}train_t;

#define FILENAME "file"

int factoryInit(pFactory_t, int, int);
int factoryStart(pFactory_t);   //启动工厂
int tcpInit(int *sFd, char* ip, char* port);
int tranFile(int newFd, char *filename);  //发送文件
//循环接受文件内容
int recvCycle(int sfd, void* pStart, int recvLen); //pStart表示接收数据的起始地址
//下载文件,接受文件名、文件大小和内容
int getFile(int socketFd, char *fileName);

//读取配置文件
int confInit(char *path, char *ip, char *port, int *threadNum, int *capacity);

//相应用户请求的命令，命令处理函数
int cmdHandle(int newFd);

//从/etc/shadow中取得的密文中取得salt
void get_salt(char *salt, char *passwd);
//响应用户的remove命令
int removeFile(int newFd, char *buf);
//得到当前路径并发送给客户端，pwd命令
int getCurrentPath(int newFd);
//列出相应目录文件,ls命令
int getCurrentList(int newFd, int, char*);
//相应用户发来的cd命令
int changeDir(char *buf);

//连接数据库
int mysqlConnect(MYSQL **connect);
//生成随机数
void Generate(char *str);
//用户注册
int userRegister(int newFd, MYSQL *conn, char *username);
#endif
