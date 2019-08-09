#define _GNU_SOURCE
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>
#include <strings.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>

#define ARGS_CHECK(argc,num) {if(argc != num) {printf("error args!\n"); return -1;}}
#define ERROR_CHECK(ret, retVal, funcName) {if(ret == retVal) {printf("errno=%d\n", errno);  perror(funcName); return -1; } }
#define PTHREAD_ERROR_CHECK(ret, funcName) { if(ret != 0) { printf("%s:%s\n", funcName, strerror(ret)); return -1; }  }

typedef struct{
    int dataLen;  //控制数据，火车头，代表后面存储的数据长度
    int command_code;  //区分用户输入的命令，六种命令，从1开始编号
    char buf[1000];
}train_t;

//循环接收文件，pStart表示接收数据的起始地址
int recvCycle(int sfd, void* pStart, int recvLen); 
//下载文件
//循环接受文件，避免ftruncate
int getFile(int socketFd, char *order2, off_t offset);
//上传文件到服务器
int putsClient(int socketFd, train_t *sendInfo, char *fileName);
//上传文件大小和文件内容
int tranFile(int socketFd, char *filename);

//下载服务器文件到本地
int getsClient(int socketFd, train_t *sendInfo, char *order2);
//客户端pwd命令
int clientPWD(int socketFd, train_t *sendInfo, train_t *recvInfo);
//cd命令
int clientCD(int socketFd, train_t *sendInfo, train_t *recvInfo, char *order2);
//ls命令
int clientLS(int socketFd, train_t *sendInfo, train_t *recvInfo, char *order2);
//remove命令
int clientRemove(int socketFd, train_t *sendInfo, train_t *recvInfo, char *order2);

//用户登录
int userLogin(int socketFd);
//用户注册
int userRegister(int socketFd);
