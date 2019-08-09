#include "func.h"

//上传文件
int tranFile(int newFd, char *filename)
{
   train_t t;
   //t.dataLen = strlen(filename);     //要转换为网络字节序，对端接收到以后，转为主机字节序
   //t.command_code = 4;
   //strcpy(t.buf, filename);
   //send(newFd, &t, 8+t.dataLen, 0);  //发送文件名
      
   int fd = open(filename, O_RDONLY);
   ERROR_CHECK(fd, -1, "open");
   //发送文件大小
   struct stat buf;
   fstat(fd, &buf);
   t.dataLen = sizeof(buf.st_size);
   t.command_code = 4;
   memcpy(t.buf, &buf.st_size, t.dataLen);
   send(newFd, &t, 8+t.dataLen, 0);

   struct timeval start, end;
   gettimeofday(&start, NULL);

   //使用mmap
   char *pMap = (char*)mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, fd, 0);
   send(newFd, pMap, buf.st_size, 0);

   //send(newFd, &t, 4+t.dataLen, 0);  //告诉客户端，文件发送
   gettimeofday(&end, NULL);
   printf("use time=%ld\n", (end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
   close(fd);
   return 0;
}

