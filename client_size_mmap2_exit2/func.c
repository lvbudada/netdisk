#include "func.h"

//下载文件,mmap接收文件
int getFile(int socketFd, char *fileName, off_t offset)
{
    int dataLen;
    //char buf[4096]={0}; 
    int command_code;
    //首先接受文件名
    //recv(socketFd, &dataLen, 4, MSG_WAITALL);
    //recv(socketFd, &command_code, 4, MSG_WAITALL);
    //recv(socketFd, buf, dataLen, MSG_WAITALL);
    //printf("%s\n", buf);

    //接受文件大小
    off_t fileSize;
    recv(socketFd, &dataLen, 4, MSG_WAITALL);
    if(0 == dataLen)
    {
        printf("文件已存在\n");
        return 0;
    }
    recv(socketFd, &command_code, 4, MSG_WAITALL);
    recv(socketFd, &fileSize, dataLen, MSG_WAITALL);
    printf("fileSize=%ld\n", fileSize);

    int fd = open(fileName, O_RDWR|O_CREAT, 0666);
    //使用splice后O_APPEND不起作用
    ERROR_CHECK(fd, -1, "open");
    //使用splice后添加的
    lseek(fd, offset, SEEK_SET);
    
    //ftruncate(fd, fileSize);
    //char *pMap = (char*)mmap(NULL, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    //ERROR_CHECK(pMap, (char*)-1, "mmap");

    int ret;
    off_t total = offset, oldFileSize = offset, spliceSize = fileSize/10000; //每次下载超过百分之0.01就打印一下进度条
    
    //while(1)
    //{
    //    ret = recv(socketFd, buf, 4096, 0);
    //    write(fd, buf, ret);
    //    total += ret;
    //    printf("%5.2f%%\r", (double)total/fileSize*100);
    //    fflush(stdout);
    //    if(total == fileSize)
    //    {
    //        break;
    //    }
    //    
    //}
    
    //使用splice零拷贝
    int fds[2];
    pipe(fds);
    while(1)
    {
        //对端关闭连接后，recv收到返回值0，splice也一样
        //在此处文件发送完，套接字并没有关闭
        ret = splice(socketFd, NULL, fds[1], NULL, 4096, SPLICE_F_MORE|SPLICE_F_MOVE);
        ERROR_CHECK(ret, -1, "splice1");
        //if(0 == ret)
        //{
        //    break;
        //}
        splice(fds[0], NULL, fd, NULL, ret, SPLICE_F_MORE|SPLICE_F_MOVE);
        ERROR_CHECK(ret, -1, "splice2");
        total += ret;
        if(total - oldFileSize >= spliceSize)
        {
            printf("%5.2f%%\r", (double)total/fileSize*100);
            fflush(stdout);
            oldFileSize = total;
        }
        if(total == fileSize)
        {
            break;
        }
    }
    printf("\n");
    //printf("%d\n", ret);
    //if(-1 == ret)
    //{
    //    printf("服务器正在升级\n");
    //    return -1;
    //}
    //munmap(pMap, fileSize);

    close(fd);
    return 0;
}

//下载服务器文件到本地，实现断点续传功能
//下载某文件时，先判断本地是否有该文件，如果存在，stat获取文件大小，传送给服务器的数据加上已存在文件的大小
int getsClient(int socketFd, train_t *sendInfo, char *order2)
{
    struct stat statBuf;
    memset(&statBuf, 0, sizeof(statBuf));
    //判断本地是否有该文件
    int ret = access(order2, F_OK);
    if(0 == ret) //文件存在
    {
        ret = stat(order2, &statBuf);
        ERROR_CHECK(ret, -1, "stat");
    }
    //将文件名称和大小写入buf里,如果不存在,文件大小为0
    sprintf(sendInfo->buf, "%s %ld", order2, statBuf.st_size);
    //strcpy(sendInfo->buf, order2); //复制要下载的文件名
    sendInfo->dataLen = strlen(sendInfo->buf);
    sendInfo->command_code = 4;
    send(socketFd, sendInfo, 8+sendInfo->dataLen, 0);
    //getFile(socketFd);
    //循环接受文件，避免ftruncate
    getFile(socketFd, order2, statBuf.st_size);
    return  0;
}

//上传文件到服务器
int putsClient(int socketFd, train_t *sendInfo, char *fileName)
{
    strcpy(sendInfo->buf, fileName);
    sendInfo->dataLen = strlen(fileName);
    sendInfo->command_code = 3;
    send(socketFd, sendInfo, 8+sendInfo->dataLen, 0);
    
    int ret;
    ret = tranFile(socketFd, fileName);
    if(!ret)
    {
        printf("upload success\n");
    }
    return 0;
}

//cd命令
int clientCD(int socketFd, train_t *sendInfo, train_t *recvInfo, char *order2)
{
    sendInfo->dataLen = strlen(order2);
    sendInfo->command_code = 1;
    strcpy(sendInfo->buf, order2);
    send(socketFd, sendInfo, 8+sendInfo->dataLen, 0);

    //接受服务器传来的消息
    clientPWD(socketFd, sendInfo, recvInfo);
    return 0;
}

//ls命令
int clientLS(int socketFd, train_t *sendInfo, train_t *recvInfo, char *order2)
{
    sendInfo->dataLen = strlen(order2);
    sendInfo->command_code = 2;
    strcpy(sendInfo->buf, order2);
    send(socketFd, sendInfo, 8+sendInfo->dataLen, 0);

    //接受服务器传来的数据
    while(1)
    {
        recv(socketFd, &recvInfo->dataLen, 4, MSG_WAITALL);
        if(0 == recvInfo->dataLen)  //当前目录下每条信息发送完成，或者当前目录下没有文件
        {
            break;
        }
        recv(socketFd, &recvInfo->command_code, 4, MSG_WAITALL);
        if(2 == recvInfo->command_code)
        {
            recv(socketFd, recvInfo->buf, recvInfo->dataLen, MSG_WAITALL);
            printf("%s\n", recvInfo->buf);
        }
    }
    return 0;
}

//remove命令
int clientRemove(int socketFd, train_t *sendInfo, train_t *recvInfo, char *order2)
{
    sendInfo->dataLen = strlen(order2);
    sendInfo->command_code = 5;
    strcpy(sendInfo->buf, order2);
    send(socketFd, sendInfo, 8+sendInfo->dataLen, 0);

    //接受服务器传来的数据
    recv(socketFd, &recvInfo->dataLen, 4, MSG_WAITALL);
    recv(socketFd, &recvInfo->command_code, 4, MSG_WAITALL);
    if(5 == recvInfo->command_code)
    {
        recv(socketFd, recvInfo->buf, recvInfo->dataLen, MSG_WAITALL);
        //printf("\033c");
        printf("%s\n", recvInfo->buf);
    }
    return 0;
}

//客户端打出的pwd命令，并接受返回的数据
int clientPWD(int socketFd, train_t *sendInfo, train_t *recvInfo)
{
    sendInfo->command_code = 6;
    send(socketFd, sendInfo, 8+sendInfo->dataLen, 0);

    //接受服务器传来的数据
    recv(socketFd, &recvInfo->dataLen, 4, MSG_WAITALL);
    recv(socketFd, &recvInfo->command_code, 4, MSG_WAITALL);
    if(6 == recvInfo->command_code)
    {
        recv(socketFd, recvInfo->buf, recvInfo->dataLen, MSG_WAITALL);
        //printf("\033c");
        printf("%s\n", recvInfo->buf);
    }
    return 0;
}

