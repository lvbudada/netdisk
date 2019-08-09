#include "factory.h"

int tranFile(int newFd, char *fileBuf)
{
   train_t t;
   //t.dataLen = strlen(filename);     //要转换为网络字节序，对端接收到以后，转为主机字节序
   //t.command_code = 4;
   //strcpy(t.buf, filename);
   //send(newFd, &t, 8+t.dataLen, 0);  //发送文件名
   char fileName[20] = {0};
   off_t offset;  //客户端已存在文件的大小
   sscanf(fileBuf, "%s %ld", fileName, &offset);
   printf("offset=%ld\n", offset);

   int fd = open(fileName, O_RDONLY);
   ERROR_CHECK(fd, -1, "open");
   //发送文件大小
   struct stat buf;
   fstat(fd, &buf);
   //判断客户端文件是否已经下载完成
   if(offset == buf.st_size)
   {
        t.dataLen = 0;
        send(newFd, &t, 4, 0);
        close(fd);
        return 0;
   }


   t.dataLen = sizeof(buf.st_size);
   t.command_code = 4;
   memcpy(t.buf, &buf.st_size, t.dataLen);
   send(newFd, &t, 8+t.dataLen, 0);

   struct timeval start, end;
   gettimeofday(&start, NULL);

   //使用mmap
   char *pMap = (char*)mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   ERROR_CHECK(pMap, (char*)-1, "mmap");
   int ret = send(newFd, pMap+offset, buf.st_size-offset, 0);
   printf("tran_file.c ret=%d\n", ret);

   //send(newFd, &t, 4+t.dataLen, 0);  //告诉客户端，文件发送
   gettimeofday(&end, NULL);
   printf("use time=%ld\n", (end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
   close(fd);
   return 0;
}

