#include "func.h"

int recvCycle(int sfd, void* pStart, int recvLen) //pStart表示接收数据的起始地址
{
    char *p = (char*)pStart;
    int total = 0;
    int ret;
    while(total<recvLen)
    {
        ret = recv(sfd, p+total, recvLen-total, 0);  //read从缓冲出读数据时，没有数据就会卡住
        if(0 == ret)
        {
            printf("\n");
            return -1;
        }
        total = total + ret;
        printf("%5.2f%%\r", (double)total/recvLen*100);
        fflush(stdout);
    }
    printf("\n");
    return 0;
}

