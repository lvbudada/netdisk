#include "func.h"

int main(int argc,char* argv[])
{
    ARGS_CHECK(argc,3);
    int socketFd;
    socketFd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(socketFd,-1,"socket");
    printf("socketFd=%d\n",socketFd);
    struct sockaddr_in serAddr;
    bzero(&serAddr,sizeof(serAddr));
    serAddr.sin_family=AF_INET;
    serAddr.sin_port=htons(atoi(argv[2]));
    serAddr.sin_addr.s_addr=inet_addr(argv[1]);
    int ret;
    ret=connect(socketFd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"connect");
    
    char getStdIn[128] = {0};
    char order1[50] = {0};
    char order2[50] = {0};
    train_t sendInfo, recvInfo;

    //用户登陆验证和用户注册
    while(1)
    {
        printf("请选择注册或者登录：0为注册，1未登录");
        scanf("%d", &ret);
        if(0 == ret)
        {
            //注册
            userRegister(socketFd); 
        }else if(1 == ret)
        {
            //登录
            ret = userLogin(socketFd);
            if(0 == ret)  //登录成功，跳出循环
            {
                break;
            }
        }
    }

    while(1)
    {
        //从标准输入得到用户输入
        memset(getStdIn, 0, sizeof(getStdIn));
        memset(order1, 0, sizeof(order1));
        memset(order2, 0, sizeof(order2));
        fgets(getStdIn, sizeof(getStdIn), stdin);
        getStdIn[strlen(getStdIn)-1] = '\0';  //去掉从标准输入读取的\n换行
        sscanf(getStdIn, "%s %s", order1, order2); //将用户输入的命令从空格分为两部分
        
        memset(&sendInfo, 0, sizeof(sendInfo));
        memset(&recvInfo, 0, sizeof(recvInfo));
        //将用户输入（用户请求）发送给服务器
        if(!strcmp("cd", order1))  //客户输入的是cd命令
        {
            clientCD(socketFd, &sendInfo, &recvInfo, order2);    
        }
        if(!strcmp("ls", order1))  //客户端输入的是pwd命令
        {
            clientLS(socketFd, &sendInfo, &recvInfo, order2);
        }
        if(!strcmp("puts", order1))  //将本地文件上传到服务器
        {
            putsClient(socketFd, &sendInfo, order2);
        }
        if(!strcmp("gets", order1))  //下载服务器文件到本地
        {
            getsClient(socketFd, &sendInfo, order2); 
        }
        if(!strcmp("remove", order1))  //remove命令
        {
           clientRemove(socketFd, &sendInfo, &recvInfo, order2);
        }
        if(!strcmp("pwd", getStdIn))  //客户端输入的是pwd命令
        {
            clientPWD(socketFd, &sendInfo, &recvInfo);
        }
       
    }

    close(socketFd);
    return 0;
}
