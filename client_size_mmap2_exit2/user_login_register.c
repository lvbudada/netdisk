#include "func.h"

int userLogin(int socketFd)
{
    char getStdIn[128] = {0};
    train_t sendInfo;
    char username[20] = {0};
    char *password;
    int ret;
    memset(&sendInfo, 0, sizeof(sendInfo));

    printf("请输入用户名:\n");
    //从标准输入得到用户名
    fgets(username, sizeof(username), stdin);
    username[strlen(username)-1] = '\0';
    //getpass函数得到用户密码
    password = getpass("请输入密码:\n");
    sprintf(getStdIn, "%s %s", username, password); //用户名和密码中间用空格隔开

    //发送给客户端用户名和密码
    sendInfo.dataLen = strlen(getStdIn);
    //command_code=1是登陆操作
    sendInfo.command_code = 1;
    strcpy(sendInfo.buf, getStdIn);
    send(socketFd, &sendInfo, 8+sendInfo.dataLen, 0);

    //接受服务器发来的数据，判断是否登录成功
    recv(socketFd, &ret, 4, MSG_WAITALL);
    if(1 == ret)
    {
        printf("登录成功\n");
        return 0;
    }else{
        printf("用户名或密码错误，请重新输入\n");
        return -1;
    }
}

//用户注册
int userRegister(int socketFd)
{
    train_t sendInfo;
    memset(&sendInfo, 0, sizeof(sendInfo));
    char username[20] = {0};
    char *password1;
    char *password2;
    int ret;
begin:
    printf("请输入用户名\n");
    scanf("%s", username);
    password1 = getpass("请输入密码\n");
    password2 = getpass("再次输入密码\n");
    if(strcmp(password1, password2))
    {
        printf("两次输入的密码不一样，请重新注册\n");
        goto begin;
    }

    //发送用户名
    strcpy(sendInfo.buf, username);
    sendInfo.dataLen = strlen(sendInfo.buf);
    sendInfo.command_code = 0;
    send(socketFd, &sendInfo, 8+sendInfo.dataLen, 0);

    //接受服务器发来的数据，判断注册是否成功
    recv(socketFd, &ret, 4, MSG_WAITALL);
    if(0 == ret)
    {
        printf("注册失败，用户名已存在，请重新注册\n");
        goto begin;
    }

    //用户名不存在，可以注册，获取salt
    char salt[20] = {0};
    char *encrypt;
    recv(socketFd, &sendInfo.dataLen, 4, MSG_WAITALL);
    recv(socketFd, &sendInfo.command_code, 4, MSG_WAITALL);
    recv(socketFd, salt, sendInfo.dataLen, MSG_WAITALL);
    //加密
    encrypt = crypt(password1, salt);
    //将加密后的密文发送给服务器
    strcpy(sendInfo.buf, encrypt);
    sendInfo.dataLen = strlen(sendInfo.buf);
    send(socketFd, &sendInfo, 8+sendInfo.dataLen, 0);
    printf("注册成功\n");

    return 0;
}
